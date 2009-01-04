#pragma once

typedef int (*sched_func_t)(void);

void sched_init(void);

void sched_loop(void);

void sched_put(sched_func_t   func);
int  sched_get(sched_func_t * func);
