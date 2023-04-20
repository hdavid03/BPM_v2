/*
 * pwm_setup.h
 *
 * Created: 4/15/2022 8:28:18 PM
 *  Author: Hajdu Dávid
 */ 


#ifndef PWM_SETUP_H_
#define PWM_SETUP_H_

#include <asf.h>

#define OC0PORT PORTC
#define OC0PINbm 0b00000100
//CTRLA:  – – – – CLKSEL[3:0]
#define DIV4 0x03
//CTRLB: CCDEN CCCEN CCBEN CCAEN – WGMODE[2:0]
//         0     1     1    0    0
#define CCCEN 6
#define CCBEN 5
#define DSTOP 0x05
#define WGMODE DSTOP
#define PWM_PERIOD (30000-1) // T = PWMPERIOD*Tclk   (Tclk = 1/12MHz)
#define pwm_max ((PWM_PERIOD + 1)/3 - 1)

#define set_pwm(x) {if(3*x > PWM_PERIOD) {tc_write_cc(&TCC0, TC_CCC, PWM_PERIOD);} \
                    else {tc_write_cc(&TCC0, TC_CCC, 3*x);}}  //0 <= pwm <=10000

void TCC0_setup(void);

#endif /* PWM_SETUP_H_ */