#pragma once

#define PIN_TLC_GSCK  PIN_15
#define PIN_TLC_SCLK  PIN_28
#define PIN_TLC_XLAT  PIN_27
#define PIN_TLC_SIN   PIN_26
#define PIN_TLC_BLNK  PIN_25
#define PIN_TLC_VPRG  PIN_24

#define PIN_DMX   PIN_INT0
#define DMX_VECT  INT0_vect

#define INT_TIMER1_COMPA tlc_int_timer1_ocma
#define INT_TIMER2_COMP  tlc_int_timer2_ocm

#define INT_TIMER0_OVF   dmx_int_timer0_ovf
#define INT_INT0         dmx_int_ext

#define PIN_DEBUG     PIN_14
