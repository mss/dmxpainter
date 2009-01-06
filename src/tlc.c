#include <avr/io.h>

#include "tlc.h"

#include "config.h"

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

void tlc_update(void);

uint8_t g_data_done;
sched_res_t wait_for_data(void)
{
  if (!g_data_done) return SCHED_RE;
  tlc_update();
  tlc_start_gscycle();
  return SCHED_OK;
}

/////////////////////////////////////////

void tlc_init(void)
{
  // Initialize blanked (ie. LEDs off).
  //set_blnk_on();

  // All these pins write to the painter.
  pin_out_off(PIN_TLC_GSCK);
  pin_out_off(PIN_TLC_VPRG);
  pin_out_off(PIN_TLC_XLAT);
  pin_out_off(PIN_TLC_SCLK);
  pin_out_off(PIN_TLC_SIN);
  pin_out_off(PIN_TLC_BLNK);

  // Initialize blanked (ie. LEDs off).
  //set_blnk_on();

  // Timer 1 is for our GSCLK:  We refresh with a GS cycle of
  // about 100 Hz (cf. Timer 2), for each full cycle we need to
  // clock the PWM 4096 times.
  // Disable output for now.
  mcu_pin_timer1_ocma_disable();
  // Shortest duty cycle possible.
  mcu_set_timer1_ocma(1);
  // We need about 39 clocks to get 4096 cycles at 100 Hz.
  mcu_set_timer1_ic(39);
  // * CS1 = 0001:  No prescaler. (p100)
  // * WGM1 = 1110: Fast PWM, TOP at ICR1
  // * COM1A = 10: Set at 0, clear at Output Compare Match)
  TCCR1B = _BV(CS10) | _BV(WGM13) | _BV(WGM12);
  TCCR1A = _BV(WGM11) | _BV(COM1A1);

  /* Timer 2: Refresh-Timer */
  // * AS2 = 0: Use IO-clock (16 MHz) for base frequency (p119)
  // * Mode: Normal mode, no PWM, count upwards (WGM21:0 = 00) (p117)
  // * Disable Output on OC2, needed for SPI (COM21:0 = 00) (p117)
  // * Prescaler: 1024 (CS22:0 = 111) => 15625 Hz
  TCCR2 = _BV(CS22) | _BV(CS21) | _BV(CS20);
  // To get a 100 Hz clock we need to count 157 times (~ 99.5 Hz).
  mcu_set_timer1_ic(156);

  // Wait for first DMX packet.
  sched_put(&wait_for_data);
}

void tlc_start(void)
{
  g_data_done = 1;
}

/////////////////////////////////////////

void tlc_start_gscycle(void)
{
  // Start counter with next GS pulse.
  mcu_int_timer1_ocma_enable();
  set_blnk_on();
}

void tlc_start_gscycle_timeout(void)
{
  // First, disable this interrupt.
  mcu_int_timer1_ocma_disable();

  // Enable PWM output.
  mcu_pin_timer1_ocma_enable();

  // Restart and enable timeout timer.
  mcu_set_timer2_cnt(0);
  mcu_int_timer2_ocm_enable();

  // Switch off BLNK.
  //set_blnk_off();
  set_blnk_on();
}

void tlc_stop_gscycle(void)
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
  for (uint8_t bit = _B(1, 0, 0, 0, 0, 0, 0, 0); bit; bit >>= 1) {
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
  for (uint8_t bit = _B(0, 0, 0, 0, 1, 0, 0, 0); bit; bit >>= 1) {
    pin_on(PIN_TLC_SCLK);
    pin_off(PIN_TLC_SCLK);
  }
}

/////////////////////////////////////////

void tlc_send_dc(void)
{
  
  for (int rgb = 2; rgb != -1; rgb--) {
    uint8_t dc_data = dc_buffer[rgb] & _B(1, 1, 1, 1, 1, 1, 0, 0);
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

void tlc_send_gs(void)
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

void tlc_update(void)
{
  // Always shift out DC first.
  tlc_send_dc();
  clock_xlat();

  // No extra SCLK needed, just shift out all GS data.
  tlc_send_gs();
  clock_xlat();
}

/////////////////////////////////////////
