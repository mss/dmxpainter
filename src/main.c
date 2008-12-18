#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "config.h"

#include "bits.h"

#define USART_RX_BUFFER_SIZE 64
#define USART_RX_TIMEOUT     10

uint8_t          g_usart_rx_buffer[USART_RX_BUFFER_SIZE];
uint8_t          g_usart_rx_buffer_out;
volatile uint8_t g_usart_rx_buffer_in;
volatile uint8_t g_usart_rx_timer;

ISR(USART_RX_vect) {
  uint8_t next;
  unsigned char data;
  
  next = (g_usart_rx_buffer_in + 1) % USART_RX_BUFFER_SIZE;
  data = UDR;

  // not full?
  if (next != g_usart_rx_buffer_out) {
    g_usart_rx_buffer[g_usart_rx_buffer_in] = data;
	g_usart_rx_buffer_in = next;
  }
  else {
    g_error |= E_USART_BUFFER_OVF;
  }
}

/* Each ms */
ISR(TIMER0_OVF_vect) {
  TCNT0 = TCNT0_RELOAD;

  if (g_usart_rx_timer)
    g_usart_rx_timer--;
}


void main_init() {
  cli();
  set_sleep_mode(SLEEP_MODE_IDLE);

  // Configure MISO, MOSI, SCK, SS for SD-access
  DDRB 

  //sd_raw_init || blink

// init_dmx
}

void main_loop() {
  while (1) {
    
  }
}

int main(void) {
  main_init();
  main_loop();
  return 0;
}
