#include <avr/io.h>

#include "tlc.h"

#include "mcu.h"

#include "buf.h"


/*********************************************************************/
/* Declaration of private constants.                                 */

#define TLC_N_CHANNELS_PER_TLC 16
#define TLC_N_TLCS_PER_PAINTER  3

#define TLC_N_CHANNELS_PER_PAINTER (TLC_N_TLCS_PER_PAINTER * TLC_N_CHANNELS_PER_TLC)

#define TLC_N_CHANNELS (N_PAINTER * TLC_N_CHANNELS_PER_PAINTER)

#define TLC_DC_ONCE


/*********************************************************************/
/* Declaration of private global variables.                          */

/**
 * Flag to indicate that data is currently shifted out.
 */
static volatile uint8_t data_shifting_;


/*********************************************************************/
/* Declaration of private functions.                                 */

static void start_gscycle(void);

static void set_shifting_off(void);
static void clock_xlat(void);
static void clock_sclk(void);
static void set_blnk_on(void);
static void set_blnk_off(void);
static void set_vprg_gs_mode(void);
static void set_vprg_dc_mode(void);

static void send_dc_data(void);
static void send_gs_data(void);


/*********************************************************************/
/* Implementation of public interrupts.                              */

/**
 * Handler for Output-Compare-Match interrupt on 16-bit timer:
 * Syncs on GSCLK to start GS cycle.
 */
void tlc_int_timer1_ocma(void)
{
  // First, disable this interrupt.
  mcu_int_timer1_ocma_disable();

  // Leave BLNK mode (switch on LEDs and start GS cycle).
  set_blnk_off();
}

/**
 * Handler for Output-Compare-Match interrupt on 8-bit timer:
 * Disables PWM when a full GSCK cycle is done.
 */
void tlc_int_timer2_ocm(void)
{
  // Go into BLNK mode (switch off LEDs and reset GSCLK counter)
  set_blnk_on();

  // Disable GS cycle timeout timer.
  mcu_int_timer2_ocm_disable();

  // Wait for next DMX packet.
  set_shifting_off();
}


/*********************************************************************/
/* Implementation of public functions.                               */

void tlc_init(void)
{
  // We have to use the 16-bit timer for the GSCLK and the 8-bit
  // timer for the timeout even though it would be better the other
  // way round:  We need the OC2 pin for SPI and thus can generate 
  // a PWM on OC1A (A because it can use ICR1 for TOP, p85) only.
  // Do this setup before enabling the output (p86).

  // Timer 1 is for our GSCLK:  We refresh with a GS cycle of
  // about 100 Hz (cf. Timer 2), ie. each 10 ms.  For each full 
  // cycle we needed to clock the PWM 4096 times which would be 
  // a count of about 39 times between the pules:
  //   n = 16 MHz / (4096 * 100 Hz) = 39.0625
  // But we have to consider the time it takes to shift out the data
  // so we don't have 10 ms for 4096 pulses, so we've got to speed 
  // up, ie. count less clocks.  The count is adapted on demand later
  // on, we'll initialize it with a reasonable default.
  mcu_set_timer1_ic(38);
  // Duty cycle as short as possible (see COM1A below).
  mcu_set_timer1_ocma(1);
  // * CS1   =  001: No prescaler. (p100)
  // * WGM1  = 1110: Fast PWM, TOP at ICR1 (p78, p89, p98)
  // * COM1A =   10: Set OC1A at 0, clear at OCM1A (p97)
  // * COM1B =   00: No output on OC1B (p97)
  TCCR1B = bits_value(CS10)
         | bits_value(WGM13)
         | bits_value(WGM12);
  TCCR1A = bits_value(WGM11)
         | bits_value(COM1A1);

  // Timer 2 is the refresh timer which determines the time one GS
  // cycle is finished; triggers Output Compare Match.
  // * AS2  =   0: Use IO-clock (16 MHz) for base frequency (p119)
  // * CS2  = 111: Use a prescaler of 1024 (p119)
  // * WGM2 =  00: Normal mode, no PWM, count upwards (p117)
  // * COM2 =  00: Disable Output on OC2, needed for SPI (p117)
  TCCR2 = bits_value(CS22)
        | bits_value(CS21)
        | bits_value(CS20);
  // With a prescaler of 1024 this timer counts at 15.625 kHz,
  // to get a 100 Hz clock we need to count 157 times (~ 99.5 Hz)
  // and refresh after that (that equals to 4 PWM pulses when
  // ignoring the shifting time).
  //   n = (16 MHz / 1024) / 100 Hz = 156.25
  mcu_set_timer2_ocm(156);

  // All these pins write to the painter.
  pin_out(PIN_TLC_GSCK);
  pin_out(PIN_TLC_VPRG);
  pin_out(PIN_TLC_XLAT);
  pin_out(PIN_TLC_SCLK);
  pin_out(PIN_TLC_SIN);

  // This one writes too, but has to be initialized blanked
  // (ie. LEDs off).  The external pullup took care against
  // flickering on boot.
  pin_out(PIN_TLC_BLNK);
  set_blnk_on();

  // Here we could read the return from the painter.
  pin_in( PIN_TLC_SRTN);
}

void tlc_exec(void)
{
  // If enabled, shift out DC once.
  #ifdef TLC_DC_ONCE
    send_dc_data();
    clock_xlat();
  #endif
}

void tlc_update(void)
{
  // Don't send anything if PWM is still active.
  if (data_shifting_) return;

  // Restart and enable 100 Hz-timeout timer now so
  // it includes the time we need to shift out data.
  mcu_set_timer2_cnt(0);
  mcu_int_timer2_ocm_enable();

  // If not disabled, always shift out DC first.
  #ifndef TLC_DC_ONCE
    send_dc_data();
    clock_xlat();
  #endif

  // No extra SCLK needed, just shift out all GS data.
  send_gs_data();
  clock_xlat();

  // A final SCLK to notify 
  clock_sclk();

  // Start PWM and continue in background...
  start_gscycle();
}


/*********************************************************************/
/* Implementation of private functions.                              */

static void set_shifting_off(void)
{
  data_shifting_ = 0;
}

// XLAT pulse to apply data to internal register.
static void clock_xlat(void)
{
  pin_on(PIN_TLC_XLAT);
  pin_off(PIN_TLC_XLAT);
}

// SCLK pulse to clock in serial data from SIN.
static void clock_sclk(void)
{
  pin_on(PIN_TLC_SCLK);
  pin_off(PIN_TLC_SCLK);
}

static void set_blnk_on(void)
{
  pin_on(PIN_TLC_BLNK);
}

static void set_blnk_off(void)
{
  pin_off(PIN_TLC_BLNK);
}

static void set_vprg_gs_mode(void)
{
  pin_off(PIN_TLC_VPRG);
}

static void set_vprg_dc_mode(void)
{
  pin_on(PIN_TLC_VPRG);
}

/*********************************************************************/

static void start_gscycle(void)
{
  data_shifting_ = 1;
  // Start counter with next GS pulse.
  mcu_int_timer1_ocma_enable();
}

/*********************************************************************/

static void shift8(uint8_t byte)
{
  // Shift out all eight bits.
  for (uint8_t bit = bits_uint8(1, 0, 0, 0, 0, 0, 0, 0); bit; bit >>= 1) {
    if (byte & bit) {
      pin_on(PIN_TLC_SIN);
    } else {
      pin_off(PIN_TLC_SIN);
    }
    clock_sclk();
  }
}

static void shift12(uint8_t byte)
{
  // The data in the upper 8 bits.
  shift8(byte);

  // Plus 4 zero bits (makes a shift by 4).
  pin_off(PIN_TLC_SIN);
  for (uint8_t bit = 4; bit; bit--) {
    clock_sclk();
  }
}

/*********************************************************************/

static void send_gs_data(void)
{
  // Set VPRG to GS mode.
  set_vprg_gs_mode();
  // Because the TLCs are daisy-chained, we have to shift out the RGB data
  // starting at the end.  Each painter has 3 TLCs (with 16 channels each), 
  // for the colors red, green, blue.  So we've got to shift out the 16 blue
  // channels of the last TLC first, then 16 green ones and finally 16 red 
  // ones.  The last data we shift out is thus the first red of the first
  // painter.
  // This will always point to the start of the current painter data, 
  // starting with the last one.
  char * painter_gs = buf_gs__
                    + TLC_N_CHANNELS
                    - TLC_N_CHANNELS_PER_PAINTER;
  // Find the current data byte to shift out, starting with the last one.
  // Its signed so we can determine when we reached the end/start, eight
  // bit are enough to index 48 channels per painter.
#if TLC_N_CHANNELS_PER_PAINTER != 48
#error What a weird painter...
#endif
  while (1) {
    int8_t index = TLC_N_CHANNELS_PER_PAINTER - 1;
    while (1) {
      // Shift out current channel.
      shift12(painter_gs[index]);

      // Skip two colors.
      index -= 3;

      // If we reached the start, we jump to the next color.
      if (index < 0) {
        // Did we just finish the last (ie. red) channel?
        if (index == -3)
          break;

        // Jump to end again and skip to next color.
        index += TLC_N_CHANNELS_PER_PAINTER - 1;
      }
    }

    // Did we just finish the last (ie. first) painter?
    if (painter_gs == buf_gs__)
      break;

    // Move to next painter.
    painter_gs -= TLC_N_CHANNELS_PER_PAINTER;
  }
}

static void send_dc_data(void)
{
  // Set VPRG to DC mode. 
  set_vprg_dc_mode();

  // All TLCs on all the connected painters will get the same DC value.
  // That makes it easy to generate the 6-Bit format we need:  We just
  // create a constant buffer for the packed rgb values, containing four
  // values for each color.
  uint8_t dc_out[3][3];
  for (int8_t rgb = 2; rgb >= 0; rgb--) {
    uint8_t dc_data = buf_dc__[rgb] & bits_uint8(1, 1, 1, 1, 1, 1, 0, 0);
    dc_out[rgb][2] = (dc_data << 0) | (dc_data >> 6);
    dc_out[rgb][1] = (dc_data << 2) | (dc_data >> 4);
    dc_out[rgb][0] = (dc_data << 4) | (dc_data >> 2);
  }

  // Now, shift out the dc-data like we do it with the gs-data:  First the
  // last blue, then green and red of the last painter, until we reach the
  // first red.
  int8_t painter = N_PAINTER;
  do {
    int8_t rgb = 3 - 1;
    do {
      int8_t index = (TLC_N_CHANNELS_PER_TLC / 4) * 3 - 1;
      do {
        shift8(dc_out[rgb][index % 3]);
        index--;
      } while (index != -1);
      rgb--;
    } while (rgb != -1);
    painter--;
  } while (painter != 0);
}

/*********************************************************************/
