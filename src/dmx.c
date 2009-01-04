#include "dmx.h"

#include "config.h"
#include "mcu.h"

#include "sched.h"

#if 0

uint8_t g_frame_count;
uint8_t g_dmx_state;
#define STATE_IDLE 0
#define STATE_WAIT 1
#define STATE_MARK 2

void dmx_start(void)
{
  g_dmx_state   = STATE_WAIT;
  g_frame_count = 0;
}



sched_res_t wait_for_mark(void)
{
  if (g_frame_count < 2) return SCHED_RE;
  
  return SCHED_OK;
}

typedef void (*state_func_t)(void);
state_func_t g_int_handler;
state_func_t g_sch_handler;
uint8_t g_data;

inline void set_state(state_func_t i, state_func_t s)
{
  g_int_handler = i;
  g_sch_handler = s;
}


// http://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
// http://blogs.sun.com/nike/entry/fast_interpreter_using_gcc_s
// http://pramode.net/2006/01/18/stackless-protothreads-using-computed-gotos/
// http://www.sics.se/~adam/pt/
// http://www.reddit.com/r/programming/comments/6ltfo/squirrelfish_webkits_new_javascript_interpreter/c047sus

enum dmx_state {
  state_idle_wait,
  state_reset_sync,
  state_reset_wait,
  state_mark_sync,
  state_mark_wait,
  state_nop
};
enum dmx_state g_state;

#define DJ(a, b) &&state_ ## b
#define DT(a, b, c, d) state_ ## a: if (d) { g_state = state_ ## b; do c while(0); } return

void dmx_int_handler(void)
{
  static void *jump[] = {
    DJ(idle_wait, idle_wait),
    DJ(reset_sync, reset_sync),
    DJ(reset_wait, reset_wait),
    DJ(mark_sync,  mark_sync),
    DJ(mark_wait,  mark_wait),
    DJ(data_start, data_start),
    DJ(data_store, nop),
    &&state_nop };
  goto *jump[g_state];
DT(idle_wait,  reset_sync, {}, 1);
DT(reset_sync, idle_wait,  {}, 1);
DT(reset_wait, mark_sync,  {}, 1);
DT(mark_sync,  idle_wait,  {}, 1);
DT(mark_wait,  data_start, {}, 1);
DT(data_start, data_store, {}, 1);
state_nop:
}

sched_res_t sched_handler(void)
{
  static void *jump[] = {
    DJ(idle_wait,  nop),
    DJ(reset_sync, reset_sync),
    DJ(reset_wait, nop),
    DJ(mark_sync,  mark_sync),
    DJ(mark_wait,  nop),
    DJ(data_start, data_start),
    DJ(data_store, nop),
    &&state_nop };
  goto *jump[g_state];
DT(reset_sync, reset_wait, {}, (g_frame_count >= 22), 1);
DT(mark_sync,  mark_wait,  {}, (g_frame_count >= 2), 1);
DT(data_store, data_store, { g_data = (g_data << 1) | pin_get(PIN_DMX); }, 1);
state_nop:
}


void dmx_init(void)
{
  // Configure as input.
  pin_in(PIN_DMX);

  g_int_handler = &wait_state;
  g_sch_handler = NULL;

  // Trigger INT0 on any edge (p67)
  _BC(MCUCR, _BV(ISC01));
  _BS(MCUCR, _BV(ISC00));

  sched_put(&sched_handler);
}

#define TIMER_BOTTOM (0xFF - 16 * 4)
void dmx_count_frame(void)
{
  g_frame_count++;
  mcu_set_timer0_cnt(TIMER_BOTTOM);
}


void start_timer(void)
{
  // 
  mcu_set_timer0_cnt(TIMER_BOTTOM);
  // CS0 = 001: Start timer 0 without prescaler (p72)
  _BS(TCCR0, _BV(CS00));
}


void dmx_int_enable()
{
  // Enable INT0 (p67)
  _BS(GICR, _BV(INT0));
}

void dmx_int_disable()
{
  // Disable INT0 (p67)
  _BC(GICR, _BV(INT0));
}



#else
void dmx_init(void) {}
#endif