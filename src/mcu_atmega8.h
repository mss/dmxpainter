#pragma once

#include "bits.h"

#define mcu_set_timer0(v) TCNT0 = v

#define mcu_set_timer1(v) TCNT1 = v
#define mcu_int_timer1_ocma_enable()  _BS(TIMSK, OCIE1A)
#define mcu_int_timer1_ocma_disable() _BC(TIMSK, OCIE1A)
#define mcu_int_timer1_ocmb_enable()  _BS(TIMSK, OCIE1A)
#define mcu_int_timer1_ocmb_disable() _BC(TIMSK, OCIE1A)

#define mcu_int_timer2_ocm_enable()   _BS(TIMSK, OCIE2);
#define mcu_int_timer2_ocm_disable()  _BC(TIMSK, OCIE2);



#define mcu_set_timer2(v) TCNT2 = v
