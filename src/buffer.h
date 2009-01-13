#pragma once

extern char gg_buffer_gs[512];
extern char gg_buffer_dc[3];

void buffer_init(void);
void buffer_next(void);

#include "sched.h"
sched_res_t buffer_test_next(void);
