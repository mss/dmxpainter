#include "dmx.h"

#include "mcu.h"

#include "buffer.h"


void enable_timer(int8_t us)
{
  // Prepare timer counter.
  TCNT0 = 0xFF - 2 * us;

  // Enable timer, use a frequency prescaler of 8 (p72).
  bits_mask_on(TCCR0, (1 << CS01));
}

void disable_timer(void)
{
  // Disable timer (p72).
  bits_mask_off(TCCR0, (1 << CS02) | (1 << CS01) | (1 << CS00));
}


void enable_trigger(void)
{
  // Enable interrupt.
  bits_on(GICR, INT0);
}

void disable_trigger(void)
{
  // Disable interrupt.
  bits_off(GICR, INT0);
}

void enable_usart(void)
{
  // Enable RXD.
  bits_on(UCSRB, RXEN);
}

void disable_usart(void)
{
  // Disable RXD.
  bits_off(UCSRB, RXEN);
}

///////////////////////////////

void dmx_init(void)
{
  // Configure both pins as input.
  pin_in(PIN_DMX_INT);
  pin_in(PIN_DMX_RXD);

  // Initialize USART (p156) with
  // 250kbaud    (UBRR = 3),
  // 8 data bits (UCSZ = 011),
  // 2 stop bits (USBS = 1),
  // no parity   (UPM  = 00).
  UBRRL = F_CPU / (16 * 250e3) - 1;
  UBRRH = (0 << URSEL) | 0;
  UCSRC = (1 << URSEL)
        | bits_value(UCSZ1) | bits_value(UCSZ0)
        | bits_value(USBS);
  // Enable USART RXD interrupt (and clear UCSZ2 and *XEN).
  UCSRB = bits_value(RXCIE);

  // Enable timer interrupt (p72).
  bits_on(TIMSK, TOIE0);

  // Trigger INT0 on any edge (ISC0 = 01, p67).
  bits_on(MCUCR, ISC00);
  enable_trigger();
}

enum {
  STATE_IDLE,
  STATE_SYNC,
  STATE_WAIT,
  STATE_RECV,
  STATE_STOR
} g_state;
int16_t g_index;


void dmx_int_timer0_ovf(void)
{
  // Disable this interrupt.
  disable_timer();

  switch (g_state) {
    case STATE_SYNC: {
      // Line was low for 88 us, all is fine.
      //mcu_debug_off();
      g_state = STATE_WAIT;
      break;
    }
    case STATE_RECV:
    case STATE_STOR: {
      // We got a timeout, back to Idle.
      disable_usart();
      enable_trigger();
      g_state = STATE_IDLE;
      break;
    }
    default: {
      break;
    }
  }
  
}

void dmx_int_ext(void)
{
  switch (g_state) {
    case STATE_IDLE: {
      // Only trigger on fallen edge.
      if (!pin_is_set(PIN_DMX_RXD)) {
        // Wait for 88 us.
        enable_timer(88);
        mcu_debug_off();
        //mcu_debug_on();
        g_state = STATE_SYNC;
      }
      break;
    }
    case STATE_SYNC: {
      // Got a stray edge while Reset, back to Idle.
      disable_timer();
      //mcu_debug_off();
      g_state = STATE_IDLE;
      break;
    }
    case STATE_WAIT: {
      // This edge is the Mark, start the USART and disable
      // this interrupt.
      disable_trigger();
      enable_usart();
            mcu_debug_on();      mcu_debug_off();
      g_state = STATE_RECV;
      break;
    }
    default: {
      break;
    }
  }
}

void dmx_int_usart_rxc(void)
{
  // Read USART data (p146).
  uint8_t rxd;
  uint8_t err;
  // Read Frame Error flag.
  err = UCSRA & bits_value(FE);
  // Read data byte (and clear RXC flag).
  rxd = UDR;
        
  if (err) {
  mcu_debug_on();      mcu_debug_off();
    goto last;
  }

  switch (g_state) {
    case STATE_RECV: {
      // Check for valid start byte.
      //if (rxd & (1 << 0)) { mcu_debug_on(); } else { mcu_debug_off(); }
      /*if (rxd != 0x00) {
        goto last;
      }*/
      
      // Switch to data storage.
      g_index = 0;
      g_state = STATE_STOR;
      break;
    }
    case STATE_STOR: {
      // Write byte to buffer.
      gg_buffer_gs[g_index] = rxd;
      // Next index.
      g_index++;
      if (g_index == 512) {
        goto last;
      }
      break;
    }
    default: {
      break;
    }
  }

  return;
last:
  // Either an invalid or the last frame appeared, stop DMX.
  disable_usart();
  enable_trigger();
  g_state = STATE_IDLE;
  return;
}
