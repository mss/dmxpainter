#include "buffer.h"

char gg_buffer_gs[512] = {
  /* R     G     B */
   0xFF, 0xFF, 0xFF,  // 01
   0xFF, 0x00, 0xFF,  // 02
   0x00, 0x00, 0x00,  // 03
   0xDD, 0xDD, 0xDD,  // 04
   0xCC, 0xCC, 0xCC,  // 05
   0xBB, 0xBB, 0xBB,  // 06
   0xAA, 0xAA, 0xAA,  // 07
   0x99, 0x99, 0x99,  // 08
   0x88, 0x88, 0x88,  // 09
   0x77, 0x77, 0x77,  // 10
   0x66, 0x66, 0x66,  // 11
   0x55, 0x55, 0x55,  // 12
   0x44, 0x44, 0x44,  // 13
   0x33, 0x33, 0x33,  // 14
   0x22, 0x22, 0x22,  // 15

   0x33, 0x00, 0x33   // 16

};

char gg_buffer_dc[3]   = {
  0x00, // R
  0x00, // G
  0x0B  // B
};


///////////////////////////////

#include "mcu.h"
#include "sched.h"
#include "tlc.h"

#include <string.h>

sched_res_t buffer_test_next(void);

volatile uint32_t g_delay;
volatile uint8_t g_rgb = 0;
void reset_counter(void)
{
  g_delay = 0;
}

void buffer_init(void)
{
#if 0
  #define BUFFER_INIT_KEEP 1
  #if BUFFER_INIT_KEEP == 0
  memset(gg_buffer_gs, 0x00, sizeof(gg_buffer_gs));
  #endif

  for (uint8_t i = 0; i < (TLC_N_CHANNELS / TLC_N_TLCS_PER_PAINTER - BUFFER_INIT_KEEP); i++)
    for (uint8_t rgb = 0; rgb < 3; rgb++)
      gg_buffer_gs[i * 3 + rgb] = 0xF0 | (rgb + 1);
   reset_counter();
#endif
}

#if 0
void buffer_next(void)
{
  //sched_put(&buffer_test_next);
  //tlc_set_data_done();
  buffer_test_next();
}

sched_res_t buffer_test_next(void)
{
  char * foo = gg_buffer_gs + 15 * 3;

  //if (g_delay++ < 100000) return 0;
  //reset_counter();
  foo[0] = (g_rgb % 3) == 0 ? 0xFF : 0x00;
  foo[1] = (g_rgb % 3) == 1 ? 0xFF : 0x00;
  foo[2] = (g_rgb % 3) == 2 ? 0xFF : 0x00;
  //g_rgb--;

  tlc_set_data_done();
  return SCHED_OK;
}

void buffer_do(void)
{
  g_rgb--;
}
#endif
