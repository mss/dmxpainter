#pragma once

#include "pins.h"

#include "mcu_config.h"

#ifdef __AVR_ATmega8__
#include "mcu_atmega8.h"
#else
#error Unknown MCU
#endif
