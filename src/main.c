// http://www.nongnu.org/avr-libc/user-manual/modules.html

#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>

//#include <util/atomic.h>

#include "config.h"

#include "bits.h"

#include "pins.h"

#include "sched.h"
#include "dmx.h"
#include "tlc.h"
#include "sd.h"


//////////////////////////////////////////
// Interrupts

// TIMER0:  8-Bit: ms for DMX, timeouts

// TIMER1: 16-Bit: GSCLK
ISR(TIMER1_COMPA_vect)
{
  tlc_start_gscycle_timeout();
}

// TIMER2:  8-Bit: GS-Refresh-Timer
ISR(TIMER2_COMP_vect)
{
  tlc_stop_gscycle();
}



// INT0:  Etxernal int, DMX sync
ISR(INT0_vect)
{
  //dmx_int_trigger();
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
