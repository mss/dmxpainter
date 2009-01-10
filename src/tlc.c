#include <avr/io.h>

#include "tlc.h"

#include "mcu.h"
#include "pins.h"

#include "sched.h"
#include "buffer.h"

/////////////////////////////////////////

#define CHANNELS_PER_TLC 16
#define BITS_PER_CHANNEL 12
#define TLCS_PER_PAINTER  3

#define BYTES_PER_CHANNEL ((CHANNELS_PER_TLC * BITS_PER_CHANNEL) / 8)

/////////////////////////////////////////

sched_res_t wait_for_data(void);

uint8_t g_data_available;

/////////////////////////////////////////

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

/////////////////////////////////////////

void tlc_init(void)
{
  // Initialize blanked (ie. LEDs off).
  pin_out(PIN_TLC_BLNK);
  pin_on( PIN_TLC_BLNK);

  // Timer 1 is for our GSCLK:  We refresh with a GS cycle of
  // about 100 Hz (cf. Timer 2), for each full cycle we need to
  // clock the PWM 4096 times.
  // Shortest duty cycle possible.
  mcu_set_timer1_ocma(1);
  // We need about 38 clocks to get 4096 cycles at 100 Hz.
  mcu_set_timer1_ic(38);
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

  // Wait for first DMX packet.
  sched_put(&wait_for_data);
}

void tlc_set_data_done(void)
{
  g_data_available = 1;
}

/////////////////////////////////////////

void start_gscycle(void)
{
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
}

void tlc_int_timer2_ocm(void)
{
  // Go into BLNK mode (switch off LEDs and reset GSCLK counter)
  set_blnk_on();

  // Disable GS cycle timeout timer.
  mcu_int_timer2_ocm_disable();

  // Wait for next DMX packet.
  sched_put(&wait_for_data);
  // TODO: next data
  sched_put(&buffer_test_next);
}

/////////////////////////////////////////

void shift8(uint8_t byte)
{
  for (uint8_t bit = bits_uint8(1, 0, 0, 0, 0, 0, 0, 0); bit; bit >>= 1) {
    if (bit & byte) {
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
  for (uint8_t bit = bits_uint8(0, 0, 0, 0, 1, 0, 0, 0); bit; bit >>= 1) {
    clock_sclk();
  }
}

/////////////////////////////////////////

void send_dc_data(void)
{
  for (int rgb = 2; rgb != -1; rgb--) {
    uint8_t dc_data = dc_buffer[rgb] & bits_uint8(1, 1, 1, 1, 1, 1, 0, 0);
    uint8_t dc_out[3] = {
      (dc_data << 0) | (dc_data >> 6),
      (dc_data << 2) | (dc_data >> 4),
      (dc_data << 4) | (dc_data >> 2)
    };

    for (int i = 0; i < N_TLC_CHANNELS; i++) {
      shift8(dc_out[i % 3]);
    }
  }
}

void send_gs_data(void)
{
  int16_t offset = N_TLC_CHANNELS - 1;
  while (1) {
    shift12(gs_buffer[offset]);

    offset -= 3;
    if (offset < 0) {
      offset += N_TLC_CHANNELS - 1; // Jump to end again, next color implicit
      if (offset != N_TLC_CHANNELS - 1 - 3)
        break;
    }
  }
}

void send_data(void)
{
  // Always shift out DC first.
  send_dc_data();
  clock_xlat();

  // No extra SCLK needed, just shift out all GS data.
  send_gs_data();
  clock_xlat();
}


/////////////////////////////////////////

sched_res_t wait_for_data(void)
{
  if (!g_data_available) return SCHED_RE;
  send_data();
  start_gscycle();
  return SCHED_OK;
}

/////////////////////////////////////////
