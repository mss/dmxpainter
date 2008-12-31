#pragma once


#include <inttypes.h>

#include "config.h"

#ifndef N_RGB_CHANNELS
#define N_RGB_CHANNELS (16 * N_PAINTER)
#endif
#define N_TLC_CHANNELS (N_RGB_CHANNELS * 3)

void tlc_init();
void tlc_start();

void tlc_update();
int  tlc_busy();
