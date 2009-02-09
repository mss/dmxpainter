#include "dmx.h"

#include "mcu.h"

#include "sched.h"

#include "buffer.h"


void set_timer(uint8_t count)
{
  // Set timer.
  TCNT0 = 0xFF - ((F_CPU / 1000000) * count);

  // Enable timer, no prescaling.
  bits_mask_on(TCCR0, 1 << CS00);
}

void set_timer_off(void)
{
  // Disable timer (p72)
  bits_mask_off(TCCR0, (1 << CS02) | (1 << CS01) | (1 << CS00));
}

void set_falling_edge_trigger(void)
{
  bits_off(MCUCR, ISC00);
}

void set_rising_edge_trigger(void)
{
  bits_on(MCUCR, ISC00);
}


void dmx_init(void)
{
  // Configure as input.
  pin_in(PIN_DMX);

  // To start, trigger INT0 on falling edge (p67),
  // we'll keep this bit set and only toggle between
  // falling and rising.
  bits_on(MCUCR, ISC01);

  // Enable INT0
  bits_on(GICR, INT0);

  // Enable timer interrupt (p72)
  bits_on(TIMSK, TOIE0);
}

enum state_enum {
  STATE_IDLE,
  STATE_WAIT,
  STATE_MARK,
  STATE_SYNC,
  STATE_READ
};

uint8_t  g_package_buffer;
uint8_t  g_package_mask;
uint16_t g_package_index;
enum state_enum g_state = STATE_IDLE;

void dmx_int_timer0_ovf(void)
{
  switch (g_state) {
    case STATE_WAIT:
      g_state = STATE_MARK;
      g_package_index = -1;
      set_falling_edge_trigger();
      break;
      
    case STATE_SYNC:
      g_state = STATE_READ;
      g_package_mask = bits_uint8(0, 0, 0, 0, 0, 0, 0, 1);
      set_timer(4);
      break;
      
    case STATE_READ:
      if (pin_is_set(PIN_DMX)) {
        bits_mask_on( g_package_buffer, g_package_mask);
      } else {
        bits_mask_off(g_package_buffer, g_package_mask);
      }
      g_package_mask <<= 1;
      if (g_package_mask == 0)
      {
        g_state = STATE_MARK;
        switch (g_package_index) {
          case -1:
            if (g_package_buffer != 0) {
              g_state = STATE_WAIT;
              set_falling_edge_trigger();
              set_timer_off();
            }
            break;
          case 511:
            g_state = STATE_WAIT;
            set_falling_edge_trigger();
            set_timer_off();
            /* FALL THROUGH */
          default:
            gg_buffer_gs[g_package_index] = g_package_buffer;
        }
        g_package_index++;
      }

    default:
      break;
  }
}

void dmx_int_ext(void)
{
  switch (g_state) {
    case STATE_IDLE:
      g_state = STATE_WAIT;
      set_timer(88);
      set_rising_edge_trigger();
      break;
      
    case STATE_WAIT:
      g_state = STATE_IDLE;
      set_timer_off();
      set_falling_edge_trigger();
      break;
      
    case STATE_MARK:
      g_state = STATE_SYNC;
      set_timer(2);
      //set_no_trigger();
      break;
      
    default:
      break;
  }
}
