#include "sched.h"

#include <inttypes.h>

#include <string.h>

#include <avr/interrupt.h>
#include <avr/sleep.h>

/////////////////////////////////////////

#define SCHED_QUEUE_LENGTH UINT8_C(10)

void *  g_sched_ring[SCHED_QUEUE_LENGTH];
uint8_t g_sched_head;
uint8_t g_sched_tail;

/////////////////////////////////////////

void sched_init()
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  
  memset(g_sched_ring, (int)NULL, sizeof(g_sched_ring));
  g_sched_head = 0;
  g_sched_tail = 0;
}

/////////////////////////////////////////

void sched_put(sched_func_t func)
{
  if (func == NULL) return;
  g_sched_ring[g_sched_tail] = func;
  g_sched_tail = (g_sched_tail + 1) % SCHED_QUEUE_LENGTH;
}

uint8_t sched_get(sched_func_t * func)
{
  *func = g_sched_ring[g_sched_head];
  if (*func == NULL) return 0;

  g_sched_ring[g_sched_head] = NULL;
  g_sched_head = (g_sched_head + 1) % SCHED_QUEUE_LENGTH;
  return 1;
}

/////////////////////////////////////////
//TODO: enqueue in front
void sched_loop()
{
  sched_func_t first;
  uint8_t      running;
  sched_func_t func;
  sched_res_t  resched;
  
  while (1) {
    // Be atomic.
    cli();

    // Peek at scheduler queue for first call.
    running = sched_get(&func);
    first = func;
    // Need a valid pointer.
    while (running) {
      // Got something, enable interrupts, disable sleep mode.
      sei();
      sleep_disable();

      // Call the queued function.
      resched = func();

      // Be atomic again.
      cli();

      // If the call wasn't successful, reschedule.
      if (resched != SCHED_OK)
        sched_put(func);
      
      // Get next pointer, exit loop if done.
      running  = sched_get(&func);
      if (func == first) {
        sched_put(func);
        running = 0;
      }
    }

    // Wait for next interrupt.
    sleep_enable();
    sei();
    //sleep_cpu();
  }
}

