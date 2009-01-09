// http://www.nongnu.org/avr-libc/user-manual/modules.html

#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>

//#include <util/atomic.h>

#include "config.h"

#include "mcu.h"

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

#if 0
// INT0:  External int, DMX sync
ISR(INT0_vect)
{
//  dmx_int_edge();
}

// TIMER0:  8-Bit: 4 us for DMX, timeouts
ISR(TIMER0_OVF_vect)
{
//  dmx_count_frame();
}
#endif


// TIMER1: 16-Bit: GSCLK
ISR(TIMER1_COMPA_vect)
{
  tlc_start_gscycle_timeout();
}




// TIMER2:  8-Bit: GS-Refresh-Timer
uint8_t g_t2_comp;
ISR(TIMER2_COMP_vect)
{
  tlc_stop_gscycle();
}


//////////////////////////////////////////

int main(void)
{
  _BS(WDTCR, WDCE);
  _BC(WDTCR, WDE);
  cli();
  
  // Initialize scheduler.
  sched_init();

  // Initialize peripherals.
  //dmx_init();
  tlc_init();

  sei();
  // Start DMX
  //dmx_start();
  // not done yet, use dummy data
  tlc_set_data_done();

  // Start scheduler.
  sched_loop();
  return 0;
}
