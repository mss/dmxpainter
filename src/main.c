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
  /*ATOMIC_BLOCK(ATOMIC_FORCEON)*/ 
}

int main(void)
{
  cli();

  sched_init();

  dmx_init();
  tlc_init();

  sei();
  // Start TLC
  tlc_start();

  // Start scheduler.
  sched_loop();
  return 0;
}
