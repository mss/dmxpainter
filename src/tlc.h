#pragma once


#include <inttypes.h>

#include "mcu_config.h"

#define N_RGB_CHANNELS (16 * N_PAINTER)
#define N_TLC_CHANNELS (N_RGB_CHANNELS * 3)

void tlc_init(void);
//void tlc_start(void);

void tlc_set_data_done(void);

void tlc_start_gscycle(void);
void tlc_start_gscycle_timeout(void);
void tlc_stop_gscycle(void);
