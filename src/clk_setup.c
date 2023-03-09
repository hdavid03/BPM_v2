/*
 * clk_setup.c
 *
 * Created: 2022.02.27. 13:29:29
 *  Author: Zoltan
 */ 

#include <asf.h>
#include "clk_setup.h"

void setup_48MHz_12MHz_clock(void)
{
	CCP=0xD8;    // Enable the clock source
	OSC.XOSCCTRL = XOSC_RANGE_12TO16 | 7; // select freq range for 16MHz external XOSC

	CCP=0xD8;    // Enable the clock source
	OSC.CTRL = OSC_XOSCEN_bm; // enable XOSC enable
	while ((OSC.STATUS & OSC_XOSCRDY_bm) == 0) {} // wait for it to become stable

	//OSC.PLLCTRL = PLL_SRC_XOSC | CONFIG_PLL0_MUL ; // 0x20 = PLLDIV (PPLL input: XOSC, PLLDIV: 3: -> frequ = 3*16 = 48 MHz)

	//OSC.CTRL |= OSC_PLLEN_bm ; // enable the PLL...
	//while( (OSC.STATUS & OSC_PLLRDY_bm) == 0 ){} // wait until it's stable

	CCP=0xD8;
	//CLK.CTRL = CLK_SCLKSEL_PLL_gc;	// The System clock source is now  PLL (48 MHz/PSADIV_1, )
	CLK.CTRL = CLK_SCLKSEL_XOSC_gc;
	
	//if ((CONFIG_SYSCLK_PSADIV != SYSCLK_PSADIV_1)
	//|| (CONFIG_SYSCLK_PSBCDIV != SYSCLK_PSBCDIV_1_1)) {
		//			CLK.PSCTRL
		//sysclk_set_prescalers(CONFIG_SYSCLK_PSADIV,CONFIG_SYSCLK_PSBCDIV);
	//}
}
