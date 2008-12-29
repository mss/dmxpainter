#pragma once

#include "bits.h"

#define _PIN(port, pin)  1 << pin
#define _PORT(port, pin) PORT ## port
#define _DDR(port, pin)  DDR  ## port

#define pinin(port)  _BC(_DDR(port), _PIN(port))
#define pinout(port) _BS(_DDR(port), _PIN(port))

#define setpin(port) _BS(_PORT(port), _PIN(port))
#define clrpin(port) _BC(_PORT(port), _PIN(port))

//////////////////////////////////////////

#ifdef __AVR_ATmega8__
#include "pins_atmega8.h"
#else
#error Unknown MCU
#endif
