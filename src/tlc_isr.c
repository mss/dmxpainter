/*volatile uint8_t  _tlc_need_xlat;
volatile uint8_t  _tlc_update_done;

ISR(TIMER1_OVF_vect)
{
  disable_xlat_pulses();
  clear_xlat_interrupt(); //XXXX Huh?

  _tlc_update_done = 1;
  _tlc_need_xlat = 0;
}

*/
