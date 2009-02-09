#include "dmx.h"

#include "mcu.h"

#include "sched.h"

#include "buffer.h"


void dmx_init(void)
{
  // Configure as input.
  pin_in(PIN_DMX);

  // Trigger INT0 on any edge (p67)
  bits_off(MCUCR, ISC01);
  bits_on (MCUCR, ISC00);

  // Enable INT0
  bits_on(GICR, INT0);
}

void dmx_int_timer0_ovf(void)
{
}

void dmx_int_ext(void)
{
  buffer_do();
}
