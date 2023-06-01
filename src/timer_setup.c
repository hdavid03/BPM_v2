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
    // CLK DIV2 -> 12 MHz / 2 = 6 MHz
    tc_write_clock_source(&TCC0, TC_CLKSEL_DIV2_gc);
    // period 6250 -> Fs = 1500 Hz -> 6000000 / 1500 = 4000
    tc_write_period(&TCC0, 8000);
}