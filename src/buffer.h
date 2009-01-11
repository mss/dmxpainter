#pragma once

#define buffer_get_gs(index)        gg_buffer_gs[index]
#define buffer_set_gs(index, value) gg_buffer_gs[index] = value
#define buffer_get_dc(index)        gg_buffer_dc[index]
#define buffer_set_dc(index, value) gg_buffer_dc[index] = value

extern char gg_buffer_gs[512];
extern char gg_buffer_dc[3];

#include "sched.h"
sched_res_t buffer_test_next(void);
