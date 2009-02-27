#pragma once

#define mcu_set_timer0_cnt(v) TCNT0 = v

#define mcu_set_timer1_cnt(v)  TCNT1 = v
#define mcu_set_timer1_ic(v)   ICR1  = v
#define mcu_set_timer1_ocma(v) OCR1A = v
#define mcu_set_timer1_ocmb(v) OCR1B = v
#define mcu_int_timer1_ovf_enable()   bits_on( TIMSK, TOIE1)
#define mcu_int_timer1_ovf_disable()  bits_off(TIMSK, TOIE1)
#define mcu_int_timer1_ocma_enable()  bits_on( TIMSK, OCIE1A)
#define mcu_int_timer1_ocma_disable() bits_off(TIMSK, OCIE1A)
#define mcu_int_timer1_ocmb_enable()  bits_on( TIMSK, OCIE1B)
#define mcu_int_timer1_ocmb_disable() bits_off(TIMSK, OCIE1B)

#define mcu_set_timer2_cnt(v)  TCNT2 = v
#define mcu_set_timer2_ocm(v)  OCR2  = v
#define mcu_int_timer2_ocm_enable()   bits_on( TIMSK, OCIE2);
#define mcu_int_timer2_ocm_disable()  bits_off(TIMSK, OCIE2);
