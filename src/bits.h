#pragma once

#include <inttypes.h>

// Set a byte bit-by-bit
#define _B(b7, b6, b5, b4, b3, b2, b1, b0) ( \
  ((uint8_t)b7 << 7) | \
  ((uint8_t)b6 << 6) | \
  ((uint8_t)b5 << 5) | \
  ((uint8_t)b4 << 4) | \
  ((uint8_t)b3 << 3) | \
  ((uint8_t)b2 << 2) | \
  ((uint8_t)b1 << 1) | \
  ((uint8_t)b0 << 0) | \
  0 )

// Set bits based on a mask
#define _BS(v, mask) (v |=  (mask))
// Clear bits based on a mask
#define _BC(v, mask) (v &= ~(mask))
