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

uint16_t _tlc_gs_idx;


/////////////////////////////////////////

// XLAT pulse to apply data to internal register.
void tlc_xlat()
{
  setpin(XLAT);
  clrpin(XLAT);
}

// SCLK pulse to clock in serial data from SIN.
void tlc_sclk()
{
  setpin(SCLK);
  clrpin(SCLK);
}

/////////////////////////////////////////

void tlc_init()
{
  // Keep blank high until the timer start.
  pinout(BLNK);
  setpin(BLNK);

  // Initialize in GS mode.
  pinout(VPRG);
  clrpin(VPRG);

  // No latch for now.
  pinout(XLAT);
  clrpin(XLAT);

  // No clock yet as well.
  pinout(SCLK);
  clrpin(SCLK);

  // And no data.
  pinout(SIN);
  clrpin(SIN);
  pinout(GSCK);
  clrpin(GSCK);

  /* All channels to zero */
  tlc_xlat();
  
  

  /* Timer 1: XLAT, BLNK */
/*  TCCR1A = _BV(COM1A0); // TODO
  TCCR1B = _BV(WGM13); // TODO
  OCR1A = 1;
  OCR1B = 2;
  ICR1 = 8192;*/

  /* Timer 2: GSCK */
  /*TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22);
  OCR2B = 0;
  OCR2A = 3;*/
}

void tlc_start()
{
/* Start */
//  _BS(TCCR1B, CS10);
//  _BS(TCCR2B, CS20);
}

/////////////////////////////////////////

void shift8(uint8_t byte)
{
  volatile uint8_t temp = byte;
  for (uint8_t bit = _B(1, 0, 0, 0, 0, 0, 0, 0); bit; bit >>= 1) {
	if (bit & byte) {
      setpin(SIN);
    } else {
      clrpin(SIN);
    }
    setpin(SCLK);
    clrpin(SCLK);
  }
}

void shift12(uint8_t byte)
{
  // The data in the upper 8 bits.
  shift8(byte);

  // Plus 4 zero bits (makes a shift by 4).
  clrpin(SIN);
  for (uint8_t bit = _B(0, 0, 0, 0, 1, 0, 0, 0); bit; bit >>= 1) {
    setpin(SCLK);
    clrpin(SCLK);
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


uint8_t _tlc_need_xlat = 0;

int tlc_update()
{
  // Wait for XLAT.
  if (_tlc_need_xlat) return 0;

  //Always shift out DC first.
  tlc_send_dc();
  
  // No extra SCLK needed, just shift out all GS data.
  tlc_send_gs();

  // Enable XLAT timer TODO
  _tlc_need_xlat = 1;
  //enable_xlat_pulses();
  //enable_xlat_interrupt();

  return 1;
}

int tlc_busy()
{
  return _tlc_need_xlat;
}

/////////////////////////////////////////

