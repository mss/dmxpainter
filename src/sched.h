#pragma once

#include <inttypes.h>

typedef uint8_t sched_res_t;
#define SCHED_OK 0
#define SCHED_RE 1
typedef sched_res_t (*sched_func_t)(void);

void sched_init(void);

void sched_loop(void);

void sched_put(sched_func_t func);
