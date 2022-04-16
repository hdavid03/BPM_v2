/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

//#define F_CPU 12000000
#include "fsm.h"
#include <string.h>
#include <stdio.h>

function control[CALC + 1];

int main (void)
{
	char str[48];
	char tmpstr[2];
	tmpstr[1] = 0;
	// static uint16_t offset, result, pwm, step, resHgmm;
	// static volatile float resfl, offsetfl, resflV;
	// pwm = 10000;
	// step = 100;
	// offset = 0;
	init_fsm(control);
	state st = INIT;
	while(1)
	{
		st = control[st]();
	}
	/* Insert application code here, after the board has been initialized. */
}
