#pragma once

#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "pins.h"

#include "mcu_config.h"

#define nop() asm volatile ("nop")

#define mcu_isr(name) ISR(name ## _vect) { INT_ ## name (); } int main(void)

#ifdef __AVR_ATmega8__
#include "mcu_atmega8.h"
#include "mcu_atmega8_pins.h"
#else
#error Unknown MCU
#endif



#define mcu_init() pin_out(PIN_DEBUG)
volatile uint8_t gg_mcu_debug;
#define mcu_debug_apply() do { if (gg_mcu_debug) { pin_on(PIN_DEBUG); } else { pin_off(PIN_DEBUG); } } while (0)
#define mcu_debug()       do { gg_mcu_debug = ~gg_mcu_debug; mcu_debug_apply(); } while (0)
#define mcu_debug_on()    do { gg_mcu_debug =             1; mcu_debug_apply(); } while (0)
#define mcu_debug_off()   do { gg_mcu_debug =             0; mcu_debug_apply(); } while (0)
