#pragma once

#include "bits.h"

//////////////////////////////////////////

#define pin_in(pin)     _BC(pin_DDR(pin), pin_PIN(pin))
#define pin_out(pin)    _BS(pin_DDR(pin), pin_PIN(pin))

#define pin_on(pin)     _BS(pin_PRT(pin), pin_PIN(pin))
#define pin_off(pin)    _BC(pin_PRT(pin), pin_PIN(pin))

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
