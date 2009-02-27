#pragma once

#include <inttypes.h>

// Set a byte bit-by-bit.
#define bits_uint8(b7, b6, b5, b4, b3, b2, b1, b0) (uint8_t)( \
  ((uint8_t)b7 << 7) | \
  ((uint8_t)b6 << 6) | \
  ((uint8_t)b5 << 5) | \
  ((uint8_t)b4 << 4) | \
  ((uint8_t)b3 << 3) | \
  ((uint8_t)b2 << 2) | \
  ((uint8_t)b1 << 1) | \
  ((uint8_t)b0 << 0) | \
  0 )

// Set and clear bits based on a mask.
// Hmmm... why don't we have to take care of
// http://www.nongnu.org/avr-libc/user-manual/FAQ.html#faq_intpromote
#define bits_mask_on(var, mask)  (var |= (mask))
#define bits_mask_off(var, mask) (var &= ~(mask))

// A nicer name for a useful macor.
#define bits_value(v) _BV(v)

// Set and clear a single bit.
#define bits_on(var, bit)  bits_mask_on( var, bits_value(bit))
#define bits_off(var, bit) bits_mask_off(var, bits_value(bit))
