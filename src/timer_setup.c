#include <timer_setup.h>
#include <define_ports.h>

static int ctr = 0;

static void timer_overflow_callback(void)
{
    tc_clear_overflow(&TCC0);
}

void timer_setup(void)
{
    tc_enable(&TCC0);
    tc_set_overflow_interrupt_callback(&TCC0, timer_overflow_callback);
    tc_set_overflow_interrupt_level(&TCC0, TC_INT_LVL_LO);
    tc_set_wgm(&TCC0, TC_WG_NORMAL);
    tc_set_direction(&TCC0, TC_UP);
    // CLK DIV4 -> 16 MHz / 4 = 4 MHz
    tc_write_clock_source(&TCC0, TC_CLKSEL_DIV2_gc);
    // period 8000 -> Fs = 500 Hz -> 4000000 / 500 = 8000
    tc_write_period(&TCC0, 6250);
}