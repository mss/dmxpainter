#pragma once

#include "config.h"

#define TLC_N_CHANNELS_PER_TLC 16
#define TLC_N_TLCS_PER_PAINTER  3

#define TLC_N_CHANNELS_PER_PAINTER (TLC_N_TLCS_PER_PAINTER * TLC_N_CHANNELS_PER_TLC)

#define TLC_N_CHANNELS (N_PAINTER * TLC_N_CHANNELS_PER_PAINTER)

void tlc_init(void);
//void tlc_start(void);

void tlc_set_data_done(void);

void tlc_int_timer1_ocma(void);
void tlc_int_timer2_ocm(void);
