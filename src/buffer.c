#include "buffer.h"

char gs_buffer[512] = {
  /* R     G     B */
   0xFF, 0xFF, 0xFF,
   0xEE, 0xEE, 0xEE,
   0xDD, 0xDD, 0xDD,
   0xCC, 0xCC, 0xCC,
   0xBB, 0xBB, 0xBB,
   0xAA, 0xAA, 0xAA,
   0x99, 0x99, 0x99, 
   0x88, 0x88, 0x88, 
   0x77, 0x77, 0x77, 
   0x66, 0x66, 0x66, 
   0x55, 0x55, 0x55, 
   0x44, 0x44, 0x44, 
   0x33, 0x33, 0x33, 
   0x22, 0x22, 0x22, 
   0x11, 0x11, 0x11, 
   0x01, 0x02, 0x03,

   0xFF, 0xFF, 0xFF,
   0xEE, 0xEE, 0xEE,
   0xDD, 0xDD, 0xDD,
   0xCC, 0xCC, 0xCC,
   0xBB, 0xBB, 0xBB,
   0xAA, 0xAA, 0xAA,
   0x99, 0x99, 0x99, 
   0x88, 0x88, 0x88, 
   0x77, 0x77, 0x77, 
   0x66, 0x66, 0x66, 
   0x55, 0x55, 0x55, 
   0x44, 0x44, 0x44, 
   0x33, 0x33, 0x33, 
   0x22, 0x22, 0x22, 
   0x11, 0x11, 0x11, 
   0x01, 0x02, 0x03
};

char dc_buffer[3]   = {
  0xAB, // R
  0xCD, // G
  0xEF  // B
};


sched_res_t buffer_test_next(void)
{
  if (gs_buffer[0] != 0) {
    gs_buffer[0]--;
  } else {
    if (gs_buffer[1] != 0) {
      gs_buffer[1]--;
    } else {
      if (gs_buffer[2] != 0) {
        gs_buffer[2]--;
      } else {
        gs_buffer[0] = 0xFF;
  } } }
  tlc_start();
}