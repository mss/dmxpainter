#pragma once

#include "pins.h"

#define mcu_set_timer0_cnt(v) TCNT0 = v

#define mcu_set_timer1_cnt(v)  TCNT1 = v
#define mcu_set_timer1_ic(v)   ICR1  = v
#define mcu_set_timer1_ocma(v) OCR1B = v
#define mcu_set_timer1_ocmb(v) OCR1A = v
#define mcu_int_timer1_ocma_enable()  _BS(TIMSK, OCIE1A)
#define mcu_int_timer1_ocma_disable() _BC(TIMSK, OCIE1A)
#define mcu_int_timer1_ocmb_enable()  _BS(TIMSK, OCIE1A)
#define mcu_int_timer1_ocmb_disable() _BC(TIMSK, OCIE1A)
#define mcu_pin_timer1_ocma_enable()  pin_out(PIN_OC1A)
#define mcu_pin_timer1_ocma_disable() pin_in( PIN_OC1A)
#define mcu_pin_timer1_ocmb_enable()  pin_out(PIN_OC1B)
#define mcu_pin_timer1_ocmb_disable() pin_in( PIN_OC1B)

#define mcu_set_timer2_cnt(v)  TCNT2 = v
#define mcu_set_timer2_ic(v)   ICR2  = v
#define mcu_int_timer2_ocm_enable()   _BS(TIMSK, OCIE2);
#define mcu_int_timer2_ocm_disable()  _BC(TIMSK, OCIE2);
#define mcu_pin_timer2_ocm_enable()   pin_out(PIN_OC2)
#define mcu_pin_timer2_ocm_disable()  pin_in( PIN_OC2)
