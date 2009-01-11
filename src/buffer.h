#pragma once

extern char gg_buffer_gs[512];
extern char gg_buffer_dc[3];

#include "sched.h"
sched_res_t buffer_test_next(void);
