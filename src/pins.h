#pragma once

#include <avr/io.h>

#include "bits.h"

//////////////////////////////////////////

#define pin_is_set(pp)  (bit_is_set(pin_SFR_PIN(pp), pin_PIN_NUM(pp)))
#define pin_get(pp)     (pin_is_set(pp) >> pin_PIN_NUM(pp))

#define pin_on(pp)      bits_mask_on( pin_SFR_PRT(pp), pin_PIN_VAL(pp))
#define pin_off(pp)     bits_mask_off(pin_SFR_PRT(pp), pin_PIN_VAL(pp))

#define pin_in(pp)      bits_mask_off(pin_SFR_DDR(pp), pin_PIN_VAL(pp))
#define pin_out(pp)     bits_mask_on( pin_SFR_DDR(pp), pin_PIN_VAL(pp))

//////////////////////////////////////////

#define pin_pin(pp)  pin_PIN_NUM(pp)
#define pin_inr(pp)  pin_SFR_PIN(pp)
#define pin_outr(pp) pin_SFR_PRT(pp)
#define pin_ddr(pp)  pin_SFR_DDR(pp)

//////////////////////////////////////////

#define pin_PIN_NUM(port, pin) pin
#define pin_PIN_VAL(port, pin) bits_value(pin)
#define pin_SFR_PIN(port, pin) PIN  ## port
#define pin_SFR_PRT(port, pin) PORT ## port
#define pin_SFR_DDR(port, pin) DDR  ## port

