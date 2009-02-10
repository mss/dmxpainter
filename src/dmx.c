#include "dmx.h"

#include "mcu.h"

#include "sched.h"

#include "buffer.h"




void set_timer(int8_t ms)
{
  // Set counter to trigger correct overflow.
  TCNT0 = 0xFF - 2 * ms;

  // Enable timer interrupt (p72)
  bits_on(TIMSK, TOIE0);
}

void set_timer_off(void)
{
  // Disable timer interrupt (p72).
  bits_off(TIMSK, TOIE0);
}

void set_trigger_rising(void)
{
  // Trigger on rising edge.
  bits_on(MCUCR, ISC00);

  // Enable interrupt.
  bits_on(GICR, INT0);
}

void set_trigger_falling(void)
{
  // Trigger on falling edge.
  bits_off(MCUCR, ISC00);

  // Enable interrupt.
  bits_on(GICR, INT0);
}

void set_trigger_off(void)
{
  // Disable interrupt.
  bits_off(GICR, INT0);
}

///////////////////////////////

void dmx_init(void)
{
  // Use a frequency prescaler of 8 (p72).
  bits_mask_on(TCCR0, 1 << CS01);

  // Configure as input.
  pin_in(PIN_DMX);

  // Prepare INT0, set_trigger_* will toggle between edges.
  set_trigger_off();
  bits_on(MCUCR, ISC01);

  // Enable interrupt after a short while.
  set_timer(10);
}

///////////////////////////////

enum state_enum {
  STATE_IDLE,
  STATE_WAIT,
  STATE_MARK,
  STATE_SYNC,
  STATE_READ,
  STATE_STOR
};

uint8_t  g_package_buffer;
uint8_t  g_package_mask;
int16_t  g_package_index;
enum state_enum g_state = STATE_IDLE;

void dmx_int_timer0_ovf(void)
{
  // Disable this interrupt.
  set_timer_off();

  switch (g_state)
  {
    case STATE_IDLE: {
      // The line must be High when idle.
      if (pin_is_set(PIN_DMX)) {
        // Wait for start of Reset.
        set_trigger_falling();
      }
      else {
        // Wait for High.
        set_timer(10);
      }
      break;
    }
    
    case STATE_WAIT: {
      // Reset was held for 88 us, wait for end of Mark now.
      mcu_debug();
      set_trigger_falling();
      g_package_index = -1;
      g_state = STATE_MARK;
      break;
    }

    case STATE_SYNC: {
      // Now read the data, LSB first.
      set_timer(4);
      g_package_mask = bits_uint8(0, 0, 0, 0, 0, 0, 0, 1);
      g_state = STATE_READ;
      break;
    }

    case STATE_READ: {
      // Prepare next timer.
      set_timer(4);

      // Read bit.
      if (pin_is_set(PIN_DMX)) {
        bits_mask_on( g_package_buffer, g_package_mask);
      } else {
        bits_mask_off(g_package_buffer, g_package_mask);
      }

      // Prepare next bit.
      g_package_mask <<= 1;
      if (g_package_mask == 0) {
        // Got a full package, store it.
        set_timer(1);
        g_state = STATE_STOR;
      }
      break;
    }

    case STATE_STOR: {
      // Store byte, but skip start byte.
      if (g_package_index != -1) {
        //gg_buffer_gs[g_package_index] = g_package_buffer;
      }
      // Next byte.
      g_package_index++;

      // Wait for next package or next frame.
      if (g_package_index != 512) {
        set_trigger_falling();
        g_state = STATE_MARK;
      }
      else {
        set_timer(20);
        g_state = STATE_IDLE;
      }
      break;
    }

    default: {
      break;
    }
  }
}


void dmx_int_ext(void)
{
  // Disable this interrupt.
  set_trigger_off();

  switch (g_state)
  {
    case STATE_IDLE: {
      // Wait for end of Reset.
      set_timer(88);
      set_trigger_rising();
      g_state = STATE_WAIT;
      break;
    }

    case STATE_WAIT: {
      // We got a stray edge before Reset timeout, back to IDLE.
      set_timer(1);
      g_state = STATE_IDLE;
      break;
    }

    case STATE_MARK: {
      // We got our start bit, sync to middle.
      mcu_debug();
      set_timer(2);
      g_state = STATE_SYNC;
      break;
    }

    default: {
      break;
    }
  }
}

