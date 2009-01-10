#pragma once


#include <inttypes.h>

#include "mcu_config.h"

#define N_RGB_CHANNELS (16 * N_PAINTER)
#define N_TLC_CHANNELS (N_RGB_CHANNELS * 3)

void tlc_init(void);
//void tlc_start(void);

void tlc_set_data_done(void);

void tlc_int_timer1_ocma(void);
void tlc_int_timer2_ocm(void);
