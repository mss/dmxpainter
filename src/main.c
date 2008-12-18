#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "config.h"

#include "bits.h"

#include "pins.h"

#include "tlc.h"


//////////////////////////////////////////

void main_init() {
  cli();

  set_sleep_mode(SLEEP_MODE_IDLE);

  tlc_init();
  tlc_start();

  sei();
}

void main_loop() {
  while (1) {
    tlc_update();
    while (tlc_busy()) {}
  }
}

int main(void) {
  main_init();
  main_loop();
  return 0;
}
