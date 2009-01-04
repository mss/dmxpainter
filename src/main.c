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


// http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if __GNUC__ < 4
#error GCC 4.x.x required!
#endif

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

int main(void)
{
  cli();
  
  // Initialize scheduler.
  sched_init();

  // Initialize peripherals.
  dmx_init();
  tlc_init();

  sei();
  // Start DMX
  //dmx_start();
  // not done yet, use dummy data
  tlc_start();

  // Start scheduler.
  sched_loop();
  return 0;
}
