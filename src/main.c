// http://www.nongnu.org/avr-libc/user-manual/modules.html

#include "mcu.h"

#include "buf.h"

#include "dmx.h"
#include "tlc.h"


// We require GCC 4.x for inlining and stuff.
// http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if __GNUC__ < 4
#error GCC 4.x.x required!
#endif

/*********************************************************************/
// Interrupts

// INT0:  External int, DMX sync
mcu_register_isr(INT0);
// TIMER0:  8-Bit: 4 us for DMX, timeouts
mcu_register_isr(TIMER0_OVF);
// USART:  RXD
mcu_register_isr(USART_RXC);

// TIMER1: 16-Bit: GSCLK
mcu_register_isr(TIMER1_COMPA);
// TIMER2:  8-Bit: GS-Refresh-Timer
mcu_register_isr(TIMER2_COMP);


/*********************************************************************/

/**
 * 
 */
static inline void main_init(void)
{
  // Disable interrupts while initializing.
  cli();
  mcu_init();

  // Initialize buffer.
  buf_init();

  // Initialize peripherals.
  dmx_init();
  tlc_init();

  // Enable interrupts again.
  sei();
}

/**
 *
 */
static inline void main_exec(void)
{
  // Signal that we're running.
  pin_out(PIN_LED_ON);
  pin_on(PIN_LED_ON);
  // Start DMX processing.
  dmx_exec();
}

/**
 *
 */
static inline void main_loop(void)
{
  // Forever...
  while (1) {
    // FIXME
    // TODO: Don't store more data than necessary.
    tlc_send_data();
  }
}

int main(void)
{
  // Initialize modules.
  main_init();
  // Start modules if necessary.
  main_exec();
  // Gogogo!
  main_loop();
  // Never reached.
  return 0;
}
