// http://www.nongnu.org/avr-libc/user-manual/modules.html

#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

//#include <util/atomic.h>

#include "config.h"

#include "bits.h"

#include "pins.h"

#include "shed.h"
#include "dmx.h"
#include "tlc.h"
#include "sd.h"


//////////////////////////////////////////

//ISR(

ISR(DMX_VECT)
{
  dmx_int_trigger();
}

//////////////////////////////////////////

void main_init()
{
  /*ATOMIC_BLOCK(ATOMIC_FORCEON)*/ cli(); {
    set_sleep_mode(SLEEP_MODE_IDLE);

    sched_init();

    dmx_init();

    tlc_init();
    tlc_start();
  } sei();
}

void main_loop()
{
  sched_func_t func;
  int resched;
  while (1) {
    // Be atomic.
    cli();

    // Peek at scheduler queue for next call.
    while (sched_get(&func)) {
      // Got something, enable interrupts, disable sleep mode.
      sei();
      sleep_disable();

      // Call the queued function.
      resched = func();

      // Be atomic again.
      cli();

      // If the call wasn't successful, reschedule.
      if (resched)
        sched_put(func);
    }

    // Wait for next interrupt.
    sleep_enable();
    sei();
    sleep_cpu();
  }
}

int main(void)
{
  main_init();
  main_loop();
  return 0;
}
