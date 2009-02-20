// http://www.nongnu.org/avr-libc/user-manual/modules.html

#include "mcu.h"

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
// USART:  RXD
mcu_isr(USART_RXC);

// TIMER1: 16-Bit: GSCLK
mcu_isr(TIMER1_COMPA);
// TIMER2:  8-Bit: GS-Refresh-Timer
mcu_isr(TIMER2_COMP);


//////////////////////////////////////////

void main_init(void)
{
  cli();
  mcu_init();

  // Initialize buffer.
  buffer_init();

  // Initialize peripherals.
  //sd_init();
  dmx_init();
  tlc_init();

  sei();
}

void main_start(void)
{
  pin_on(PIN_LED_ON);
}

void main_loop(void)
{
  while (1) {
    tlc_set_data_done();
    tlc_wait_for_data();
  }
}

int main(void)
{
  main_init();
  main_start();
  main_loop();
  return 0;
}
