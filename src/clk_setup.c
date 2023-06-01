/*
 * clk_setup.c
 *
 * Created: 2022.02.27. 13:29:29
 *  Author: Zoltan
 */ 

#include <asf.h>
#include <clk_setup.h>

void setup_clock(void)
{
	ccp_write_io((uint8_t*)&OSC.XOSCCTRL, XOSC_RANGE_12TO16 | OSC_XOSCSEL_XTAL_1KCLK_gc);
	ccp_write_io((uint8_t*)&OSC.CTRL, OSC_XOSCEN_bm);
	
	while ((OSC.STATUS & OSC_XOSCRDY_bm) == 0) {} // wait for it to become stable

	OSC.PLLCTRL = PLL_SRC_XOSC | CONFIG_PLL0_MUL; // 0x20 = PLLDIV (PPLL input: XOSC, PLLDIV: 3: -> frequ = 3*16 = 48 MHz)

	OSC.CTRL |= OSC_PLLEN_bm ; // enable the PLL...
	while( (OSC.STATUS & OSC_PLLRDY_bm) == 0 ){} // wait until it's stable

	ccp_write_io((uint8_t*)&CLK.CTRL, CLK_SCLKSEL_PLL_gc);
	sysclk_set_prescalers(CONFIG_SYSCLK_PSADIV, CONFIG_SYSCLK_PSBCDIV);

}
