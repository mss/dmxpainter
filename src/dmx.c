#include "dmx.h"

#include "mcu.h"

#include "buf.h"

/*********************************************************************/
/* Declaration of private global variables.                          */

/**
 * The DMX parser is driven by a state machine using these states.
 */
enum state {
  STATE_IDLE,
  STATE_SYNC,
  STATE_WAIT,
  STATE_RECV,
  STATE_STOR
};
/**
 * The current state of the DMX state machine.
 */
static volatile enum state state_;
/**
 * Index of current DMX frame (between 0 and 512).
 */
static volatile int16_t index_;

/**
 * Current timer start value in case we want to reset.
 */
static volatile uint8_t timer_;


/*********************************************************************/
/* Declaration of private functions.                                 */

static void enable_timer(int8_t us);
static void disable_timer(void);
static void reset_timer(void);

static void enable_trigger(void);
static void disable_trigger(void);

static void enable_usart(void);
static void disable_usart(void);


/*********************************************************************/
/* Implementation of public interrupts.                              */

#define DMX_RESET_TIME   88
#define DMX_BIT_TIME     4
#define DMX_CHAR_TIME    (DMX_BIT_TIME * (8 + 3))
#define DMX_CHAR_TIMEOUT (DMX_CHAR_TIME * 2)

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
        // Wait for a Reset of 88 us (or more).
        enable_timer(DMX_RESET_TIME);
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
      // This edge is the Mark, disable this interrupt...
      disable_trigger();
      // ... and start the USART and the timeout.
      enable_usart();
      enable_timer(DMX_CHAR_TIMEOUT);
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
  // Bail out if start or stop bit was wrong.
  if (err) {
    goto finally;
  }

  // Reset timeout.
  reset_timer();

  switch (state_) {
    case STATE_RECV: {
      // Bail out if the start byte is not all low.
      if (rxd != 0x00) {
        goto finally;
      }

      // Switch to data storage.
      index_ = 0;
      state_ = STATE_STOR;
      break;
    }
    case STATE_STOR: {
      // Write byte to buffer.
      buf_gs__[index_] = rxd;
      // Jump out once we received all 512 channels.
      if (index_ == 511) {
        goto finally;
      }
      // Next index.
      index_++;
      break;
    }
    default: {
      break;
    }
  }
  return;

finally: {
    // Either an invalid or the last frame appeared, stop DMX.
    disable_timer();
    disable_usart();
    enable_trigger();
    state_ = STATE_IDLE;
    return;
  }
}


/*********************************************************************/
/* Implementation of public functions.                               */

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


/*********************************************************************/
/* Implementation of private functions.                              */

static void enable_timer(int8_t us)
{
  // Prepare timer counter.
  timer_ = 0xFF - 2 * us;
  reset_timer();

  // Enable timer, use a frequency prescaler of 8 (p72).
  bits_mask_on(TCCR0, (1 << CS01));
}

static void disable_timer(void)
{
  // Disable timer (p72).
  bits_mask_off(TCCR0, (1 << CS02) | (1 << CS01) | (1 << CS00));
}

static void reset_timer()
{
  TCNT0 = timer_;
}


static void enable_trigger(void)
{
  // Enable interrupt triggered by edge on pin.
  bits_on(GICR, INT0);
}

static void disable_trigger(void)
{
  // Disable interrupt triggered by edge on pin.
  bits_off(GICR, INT0);
}

static void enable_usart(void)
{

  // Enable RXD.
  bits_on(UCSRB, RXEN);
}

static void disable_usart(void)
{
  // Disable RXD.
  bits_off(UCSRB, RXEN);
}
