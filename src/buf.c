#include "buf.h"

/*
 * All channel greyscale data is initialized to zero.
 */
volatile char buf_gs__[512];

/*
 * One dot correction for everything.
 */
#define BUF_DC_MAX 0x3F
#define BUF_DC_R   BUF_DC_MAX
#define BUF_DC_G   0x12
#define BUF_DC_B   0x12
const char buf_dc__[3] = { BUF_DC_R, BUF_DC_G, BUF_DC_B };
