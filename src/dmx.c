#if 0
#include "dmx.h"

#include "config.h"
#incude  "pins.h"


void dmx_init()
{
  // Configure as input.
  pinin(DMX_PIN);
  // Trigger INT0 on any edge (p67)
  _BC(MCUCR, _BV(ISC01));
  _BS(MCUCR, _BV(ISC00));
  
}

//int dmx_get_

void dmx_int_enable()
{
  // Enable INT0 (p67)
  _BS(GICR, _BV(INT0));
}

void dmx_int_disable()
{
  // Disable INT0 (p67)
  _BC(GICR, _BV(INT0));
}

void dmx_wait_enable()
{
  // Wait for any edge.
  dmx_int_enable();
}



void dmx_wait_disable()
{
}
#endif