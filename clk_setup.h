/*
 * clk_setup.h
 *
 * Created: 2022.02.27. 13:33:54
 *  Author: Zoltan
 */ 


#ifndef CLK_SETUP_H_
#define CLK_SETUP_H_

#define F_CPU 12000000

// External oscillator frequency range
#define CONFIG_XOSC_RANGE XOSC_RANGE_12TO16

// USB clock
#define CONFIG_USBCLK_SOURCE                USBCLK_SRC_PLL

// CPU clock, 16MHz*3/(2*2) = 12 MHz
//#define CONFIG_SYSCLK_SOURCE				SYSCLK_SRC_XOSC
#define CONFIG_SYSCLK_PSADIV				SYSCLK_PSADIV_2
#define CONFIG_SYSCLK_PSBCDIV				SYSCLK_PSBCDIV_1_2

void setup_48MHz_12MHz_clock(void);

#endif /* CLK_SETUP_H_ */