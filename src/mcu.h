#pragma once

#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "util/pins.h"

#include "config.h"

/**
 * Execute a single NOP instruction to delay something by one cycle.
 */
#define nop() asm volatile ("nop")

/**
 * Map the ISR NAME_vect to the interrupt function INT_NAME (from config.h).
 */
#define mcu_register_isr(name) \
  void INT_ ## name (void); \
  ISR(name ## _vect) { INT_ ## name (); } \
  int main(void)


#ifdef __AVR_ATmega8__
#define  MCU "atmega8"
#include "mcu/atmega8.h"
#include "mcu/atmega8_pins.h"
#else
#error Unknown MCU
#endif


#ifdef PIN_DEBUG
#define mcu_init() do { pin_out(PIN_DEBUG); mcu_debug__ = 0; } while (0)
#ifdef REG_DEBUG_FLAG
#define mcu_debug__ REG_DEBUG_FLAG
#else
volatile uint8_t mcu_debug__;
#endif
#define mcu_debug_set(v) do { mcu_debug__ = v; if (v) { pin_on(PIN_DEBUG); } else { pin_off(PIN_DEBUG); } } while (0)
#define mcu_debug()      mcu_debug_set(~mcu_debug__)
#else
#define mcu_init()
#define mcu_debug_set(v)
#define mcu_debug()      mcu_debug_set(0xFF)
#endif
#define mcu_debug_on()   mcu_debug_set(1)
#define mcu_debug_off()  mcu_debug_set(0)
