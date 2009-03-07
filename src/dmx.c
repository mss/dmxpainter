#include "dmx.h"

#include "mcu.h"

#include "buf.h"


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
  // Enable interrupt triggered by edge on pin.
  bits_on(GICR, INT0);
}

void disable_trigger(void)
{
  // Disable interrupt triggered by edge on pin.
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
  // But explicitly disable it for now.
  disable_trigger();
}

void dmx_exec(void)
{
  // Just enable the trigger for the pin.
  enable_trigger();
}

///////////////////////////////

static enum state {
  STATE_IDLE,
  STATE_SYNC,
  STATE_WAIT,
  STATE_RECV,
  STATE_STOR
};
static volatile enum state state_;
static          int16_t index_;


void dmx_int_timer0_ovf(void)
{
  // Disable this interrupt.
  disable_timer();

  switch (state_) {
    case STATE_SYNC: {
      // Line was low for 88 us, all is fine.
      state_ = STATE_WAIT;
      break;
    }
    case STATE_RECV:
    case STATE_STOR: {
      // We got a timeout, back to Idle.
      disable_usart();
      enable_trigger();
      state_ = STATE_IDLE;
      break;
    }
    default: {
      break;
    }
  }
  
}

void dmx_int_ext(void)
{
  switch (state_) {
    case STATE_IDLE: {
      // Only trigger on fallen edge.
      if (!pin_is_set(PIN_DMX_RXD)) {
        // Wait for 88 us.
        enable_timer(88);;
        state_ = STATE_SYNC;
      }
      break;
    }
    case STATE_SYNC: {
      // Got a stray edge while Reset, back to Idle.
      disable_timer();
      state_ = STATE_IDLE;
      break;
    }
    case STATE_WAIT: {
      // This edge is the Mark, start the USART and disable
      // this interrupt.
      disable_trigger();
      enable_usart();
      state_ = STATE_RECV;
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
    goto last;
  }

  switch (state_) {
    case STATE_RECV: {
      // TODO: Check for valid start byte.
      /*if (rxd != 0x00) {
        goto last;
      }*/

      // Switch to data storage.
      index_ = 0;
      state_ = STATE_STOR;
      break;
    }
    case STATE_STOR: {
      // Write byte to buffer.
      buf_gs__[index_] = rxd;
      // Next index.
      index_++;
      // TODO: Don't store more data than necessary.
      if (index_ == 512) {
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
  state_ = STATE_IDLE;
  return;
}
