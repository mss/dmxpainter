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
