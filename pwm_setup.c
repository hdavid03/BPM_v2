/*
 * pwm_setup.c
 *
 * Created: 4/15/2022 8:28:40 PM
 *  Author: Hajdu Dávid
 */

#include "pwm_setup.h"

uint16_t PWM_TH = 0;    // THpwm = PWM_Time*Tclk

void TCC0_setup(void)
{
	TCC0.CTRLA = DIV4;	//prescaler 4, fclk = 12MHz/4 = 3 MHz
	TCC0.CTRLB = (1 << CCCEN) | (1 << CCBEN) | WGMODE;  //OC0C and OC0B enabled, and dual slope PWM with OVF on TOP
	tc_enable(&TCC0);
	tc_set_wgm(&TCC0, TC_WG_SS);
	tc_write_period(&TCC0, PWM_PERIOD);				//PWM fequency: 3e6/2*(3e4) = 50 Hz
	tc_write_cc(&TCC0, TC_CCC, PWM_TH);			// duty cycle: PWM_TH/PWM_PERIOD
	tc_enable_cc_channels(&TCC0,TC_CCCEN);
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV4_gc);
	OC0PORT.DIRSET = OC0PINbm;					//OC0PORT set as output
	set_pwm(10000);
}
