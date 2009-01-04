#pragma once

extern char gs_buffer[512];
extern char dc_buffer[3];

#include "sched.h"
sched_res_t buffer_test_next(void);
