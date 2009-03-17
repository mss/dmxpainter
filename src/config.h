#pragma once

// Settings
#define N_PAINTER 8

#define TLC_DC_ONCE 1
#define TLC_USE_REG 1

#define DMX_USE_REG 1


// MCU setup
#define PIN_TLC_GSCK  PIN_15
#define PIN_TLC_SCLK  PIN_28
#define PIN_TLC_XLAT  PIN_27
#define PIN_TLC_SIN   PIN_26
#define PIN_TLC_BLNK  PIN_25
#define PIN_TLC_VPRG  PIN_24
#define PIN_TLC_SRTN  PIN_23

#define PIN_DMX_INT   PIN_INT0
#define PIN_DMX_RXD   PIN_RXD

#define PIN_LED_ON    PIN_3

#define PIN_DEBUG     PIN_14

#define INT_TIMER1_COMPA tlc_int_timer1_ocma
#define INT_TIMER2_COMP  tlc_int_timer2_ocm

#define INT_TIMER0_OVF   dmx_int_timer_ovf
#define INT_INT0         dmx_int_ext_edge
#define INT_USART_RXC    dmx_int_usart_rxc

#define REG_DEBUG_FLAG EEARH
#define REG_TLC_STATUS EEARL
#define REG_DMX_STATE  EEDR


// Defaults
#ifndef N_PAINTER
  #define N_PAINTER 8
#elif (N_PAINTER < 1) || (N_PAINTER > 10)
  #error This will not work.
#endif

#if TLC_DC_ONCE == 0
  #undef TLC_DC_ONCE
#endif
#if TLC_USE_REG == 0
  #undef REG_TLC_STATUS
#endif

#if DMX_USE_REG == 0
  #undef REG_DMX_STATUS
#endif

#ifdef NDEBUG
  #undef DEBUG
  #undef PIN_DEBUG
#else
  #define DEBUG
#endif
