#pragma once

#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "util/pins.h"

#include "config.h"

#define nop() asm volatile ("nop")

#define mcu_isr(name) ISR(name ## _vect) { INT_ ## name (); } int main(void)

#ifdef __AVR_ATmega8__
#define  MCU "atmega8"
#include "mcu/atmega8.h"
#include "mcu/atmega8_pins.h"
#else
#error Unknown MCU
#endif



#define mcu_init() pin_out(PIN_DEBUG)
volatile uint8_t gg_mcu_debug;
#define mcu_debug_set(v) do { gg_mcu_debug = v; if (v) { pin_on(PIN_DEBUG); } else { pin_off(PIN_DEBUG); } } while (0)
#define mcu_debug()      mcu_debug_set(~gg_mcu_debug)
#define mcu_debug_on()   mcu_debug_set(1)
#define mcu_debug_off()  mcu_debug_set(0)
