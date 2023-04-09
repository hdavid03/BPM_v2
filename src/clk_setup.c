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
	// init sysclk with default configuration (conf_clock.h)
	sysclk_init();
}
