#include <avr/io.h>

#include "tlc.h"

#include "mcu.h"

#include "buf.h"

/*********************************************************************/

static volatile uint8_t data_available_;

/*********************************************************************/

static volatile uint8_t data_shifting_;
void send_data(void);
void start_gscycle(void);
void tlc_wait_for_data()
{
  if (data_shifting_) return;
  send_data();
  start_gscycle();
  // Continue in background...
}
void set_shifting_off(void)
{
  data_shifting_ = 0;
}

/*********************************************************************/

// XLAT pulse to apply data to internal register.
void clock_xlat(void)
{
  pin_on(PIN_TLC_XLAT);
  pin_off(PIN_TLC_XLAT);
}

// SCLK pulse to clock in serial data from SIN.
void clock_sclk(void)
{
  pin_on(PIN_TLC_SCLK);
  pin_off(PIN_TLC_SCLK);
}

void set_blnk_on(void)
{
  pin_on(PIN_TLC_BLNK);
}

void set_blnk_off(void)
{
  pin_off(PIN_TLC_BLNK);
}

void set_vprg_gs_mode(void)
{
  pin_off(PIN_TLC_VPRG);
}

void set_vprg_dc_mode(void)
{
  pin_on(PIN_TLC_VPRG);
}

/*********************************************************************/

void tlc_init(void)
{
  // Timer 1 is for our GSCLK:  We refresh with a GS cycle of
  // about 100 Hz (cf. Timer 2), for each full cycle we need to
  // clock the PWM 4096 times.
  // We need about 38 clocks to get 4096 cycles at 100 Hz.
  mcu_set_timer1_ic(38);
  // 50% duty cycle.
  mcu_set_timer1_ocma(1);
  // * CS1 = 0001:  No prescaler. (p100)
  // * WGM1 = 1110: Fast PWM, TOP at ICR1
  // * COM1A = 10: Set at 0, clear at Output Compare Match)
  TCCR1B = bits_value(CS10) | bits_value(WGM13) | bits_value(WGM12);
  TCCR1A = bits_value(WGM11) | bits_value(COM1A1);

  /* Timer 2: Refresh-Timer */
  // * AS2 = 0: Use IO-clock (16 MHz) for base frequency (p119)
  // * Mode: Normal mode, no PWM, count upwards (WGM21:0 = 00) (p117)
  // * Disable Output on OC2, needed for SPI (COM21:0 = 00) (p117)
  // * Prescaler: 1024 (CS22:0 = 111) => 15625 Hz
  TCCR2 = bits_value(CS22) | bits_value(CS21) | bits_value(CS20);
  // To get a 100 Hz clock we need to count 157 times (~ 99.5 Hz).
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

void tlc_set_data_done(void)
{
  data_available_ = 1;
}

/*********************************************************************/

void start_gscycle(void)
{
  data_shifting_ = 1;
  // Start counter with next GS pulse.
  mcu_int_timer1_ocma_enable();
}

void tlc_int_timer1_ocma(void)
{
  // First, disable this interrupt.
  mcu_int_timer1_ocma_disable();

  // Restart and enable timeout timer.
  mcu_set_timer2_cnt(0);
  mcu_int_timer2_ocm_enable();

  // Switch off BLNK.
  set_blnk_off();
  // Hack: Switch on GSCLK
  pin_out(PIN_TLC_GSCK);
}

void tlc_int_timer2_ocm(void)
{
  // Hack: Switch off GSCLK
  pin_in(PIN_TLC_GSCK);
  // Go into BLNK mode (switch off LEDs and reset GSCLK counter)
  set_blnk_on();

  // Disable GS cycle timeout timer.
  mcu_int_timer2_ocm_disable();

  // Wait for next DMX packet.
  set_shifting_off();
}

/*********************************************************************/

void shift8(uint8_t byte)
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

void shift12(uint8_t byte)
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

void send_gs_data(void)
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

void send_dc_data(void)
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

void send_data(void)
{
  // Always shift out DC first.
  send_dc_data();
  clock_xlat();

  // No extra SCLK needed, just shift out all GS data.
  send_gs_data();
  clock_xlat();

  clock_sclk();

  data_available_ = 0;
}


/*********************************************************************/
