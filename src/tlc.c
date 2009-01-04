#include <avr/io.h>

#include "tlc.h"

#include "config.h"

#include "pins.h"

#include "buffer.h"

/////////////////////////////////////////

#define CHANNELS_PER_TLC 16
#define BITS_PER_CHANNEL 12
#define TLCS_PER_PAINTER  3

#define BYTES_PER_CHANNEL ((CHANNELS_PER_TLC * BITS_PER_CHANNEL) / 8)

/////////////////////////////////////////

// XLAT pulse to apply data to internal register.
void tlc_xlat()
{
  setpin(PIN_TLC_XLAT);
  clrpin(PIN_TLC_XLAT);
}

// SCLK pulse to clock in serial data from SIN.
void tlc_sclk()
{
  setpin(PIN_TLC_SCLK);
  clrpin(PIN_TLC_SCLK);
}

/////////////////////////////////////////

void tlc_init()
{
  // Keep blank high until the timer start.
  pinout(PIN_TLC_BLNK);
  setpin(PIN_TLC_BLNK);

  // Initialize in GS mode.
  pinout(PIN_TLC_VPRG);
  clrpin(PIN_TLC_VPRG);

  // No latch for now.
  pinout(PIN_TLC_XLAT);
  clrpin(PIN_TLC_XLAT);

  // No clock yet as well.
  pinout(PIN_TLC_SCLK);
  clrpin(PIN_TLC_SCLK);

  // And no data.
  pinout(PIN_TLC_SIN);
  clrpin(PIN_TLC_SIN);
  pinout(PIN_TLC_GSCK);
  clrpin(PIN_TLC_GSCK);

  /* All channels to zero */
  tlc_xlat();

  /* Timer 2: Refresh-Timer */
  // * AS2 = 0: Use IO-clock (16 MHz) for base frequency (p119)
  // * Mode: Normal mode, no PWM, count upwards (WGM21:0 = 00) (p117)
  // * Disable Output on OC2, needed for SPI (COM21:0 = 00) (p117)
  // * Prescaler: 1024 (CS22:0 = 111) => 15625 Hz
  TCCR2 = _BV(CS22) | _BV(CS21) | _BV(CS20);
  // To get a 100 Hz clock we need to count 157 times (~ 99.5 Hz).
  OCR2  = 156;

  /* Timer 1: GSCLK-Timer */
  // * WGM1 = 1110: Fast PWM, TOP at ICR1
  // * COM1A = 10: Set at 0, clear at Compare Match)
  TCCR1B = _BV(WGM13) | _BV(WGM12);
  TCCR1A = _BV(WGM11) | _BV(COM1A1);
  // Shortest duty cycle possible.
  OCR1A = 1;
  // We need about 39 clocks to get 4096 cycles at 100 Hz.
  ICR1 = 39;
}

void tlc_start()
{
/* Start */
//  _BS(TCCR1B, CS10);
//  _BS(TCCR2B, CS20);
}

/////////////////////////////////////////

void tlc_start_gscycle()
{
  // Sync with next GSCLK.
  _BS(TIMSK, OCIE1A);
}

void tlc_start_gscycle_timeout()
{
  // Disable this interrupt.
  _BC(TIMSK, OCIE1A);

  // Restart timer.
  TCNT2 = 0;
  // Enable Compare Match Interrupt
  _BS(TIMSK, OCIE2);
}

void tlc_stop_gscycle()
{
  // TODO: stop timer2, stop timer1
  // TODO: blank
  // TODO: next data
}

/////////////////////////////////////////

void shift8(uint8_t byte)
{
  for (uint8_t bit = _B(1, 0, 0, 0, 0, 0, 0, 0); bit; bit >>= 1) {
	if (bit & byte) {
      setpin(PIN_TLC_SIN);
    } else {
      clrpin(PIN_TLC_SIN);
    }
    setpin(PIN_TLC_SCLK);
    clrpin(PIN_TLC_SCLK);
  }
}

void shift12(uint8_t byte)
{
  // The data in the upper 8 bits.
  shift8(byte);

  // Plus 4 zero bits (makes a shift by 4).
  clrpin(PIN_TLC_SIN);
  for (uint8_t bit = _B(0, 0, 0, 0, 1, 0, 0, 0); bit; bit >>= 1) {
    setpin(PIN_TLC_SCLK);
    clrpin(PIN_TLC_SCLK);
  }
}

/////////////////////////////////////////

void tlc_send_dc()
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

void tlc_send_gs()
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


uint8_t _tlc_busy = 0;

void tlc_update()
{
  if (_tlc_busy) return;

  // Always shift out DC first.
  tlc_send_dc();
  tlc_xlat();

  // No extra SCLK needed, just shift out all GS data.
  tlc_send_gs();
  tlc_xlat();
}

int tlc_busy()
{
  return _tlc_busy;
}

/////////////////////////////////////////

void tlc_update_done()
{
  _tlc_busy = 0;
}

/////////////////////////////////////////
