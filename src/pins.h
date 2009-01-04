#pragma once

#include <avr/io.h>

#include "bits.h"

//////////////////////////////////////////

#define pin_is_set(pin)  (bit_is_set(pin_SFR_PIN(pin), pin_PIN_NUM(pin)))
#define pin_get(pin)     (pin_is_set(pin) >> pin_PIN_NUM(pin))

#define pin_on(pin)      _BS(pin_SFR_PRT(pin), pin_PIN_VAL(pin))
#define pin_off(pin)     _BC(pin_SFR_PRT(pin), pin_PIN_VAL(pin))

#define pin_in(pin)      _BC(pin_SFR_DDR(pin), pin_PIN_VAL(pin))
#define pin_out(pin)     _BS(pin_SFR_DDR(pin), pin_PIN_VAL(pin))

#define pin_out_on(pin)  _BS(pin_SFR_DDR(pin), pin_PIN_VAL(pin)); \
                         _BS(pin_PIN_PRT(pin), pin_PIN_VAL(pin))
#define pin_out_off(pin) _BS(pin_SFR_DDR(pin), pin_PIN_VAL(pin)); \
                         _BC(pin_SFR_PRT(pin), pin_PIN_VAL(pin))

//////////////////////////////////////////

#define pin_pin(pin)  pin_PIN_NUM(pin)
#define pin_inr(pin)  pin_SFR_PIN(pin)
#define pin_outr(pin) pin_SFR_PRT(pin)
#define pin_ddr(pin)  pin_SFR_DDR(pin)

//////////////////////////////////////////

#define pin_PIN_NUM(port, pin) pin
#define pin_PIN_VAL(port, pin) _BV(pin)
#define pin_SFR_PIN(port, pin) PIN  ## port
#define pin_SFR_PRT(port, pin) PORT ## port
#define pin_SFR_DDR(port, pin) DDR  ## port

//////////////////////////////////////////

#ifdef __AVR_ATmega8__
#include "pins_atmega8.h"
#else
#error Unknown MCU
#endif
