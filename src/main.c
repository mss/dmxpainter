// http://www.nongnu.org/avr-libc/user-manual/modules.html

#include "mcu.h"

#include "sched.h"
#include "dmx.h"
#include "tlc.h"
#include "sd.h"

#include "buffer.h"


// http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if __GNUC__ < 4
#error GCC 4.x.x required!
#endif

//////////////////////////////////////////
// Interrupts

// INT0:  External int, DMX sync
mcu_isr(INT0);

// TIMER0:  8-Bit: 4 us for DMX, timeouts
mcu_isr(TIMER0_OVF);

// TIMER1: 16-Bit: GSCLK
mcu_isr(TIMER1_COMPA);
// TIMER2:  8-Bit: GS-Refresh-Timer
mcu_isr(TIMER2_COMP);


//////////////////////////////////////////

int main(void)
{
  cli();
  
  // Initialize scheduler.
  sched_init();

  // Initialize peripherals.
  //dmx_init();
  tlc_init();

  for (uint8_t i = 0; i < 15; i++)
    for (uint8_t rgb = 0; rgb < 3; rgb++)
      gg_buffer_gs[i * 3 + rgb] = 0x10 | (rgb + 1);


  sei();
  // Start DMX
  //dmx_start();
  // not done yet, use dummy data
  tlc_set_data_done();

  // Start scheduler.
  sched_loop();
  return 0;
}
