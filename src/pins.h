#pragma once

#include "bits.h"

//////////////////////////////////////////

#define pin_in(port)     _BC(pin_DDR(port), pin_PIN(port))
#define pin_out(port)    _BS(pin_DDR(port), pin_PIN(port))

#define pin_on(port)     _BS(pin_PRT(port), pin_PIN(port))
#define pin_off(port)    _BC(pin_PRT(port), pin_PIN(port))

//////////////////////////////////////////

#define pin_PIN(port, pin) 1 << pin
#define pin_PRT(port, pin) PORT ## port
#define pin_DDR(port, pin) DDR  ## port

//////////////////////////////////////////

#ifdef __AVR_ATmega8__
#include "pins_atmega8.h"
#else
#error Unknown MCU
#endif
