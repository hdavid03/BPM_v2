/**
 * \file
 *
 * \brief Blood pressure monitoring application
 *
 */

/**
 * \mainpage Blood pressure monitoring application documentation
 *
 * \par Content
 *
 * -# This application is running only AVR microcontroller with
 * 	  specific hardware setup. The blood pressure monitoring algorithm
 * 	  is based on the fixed-ratio method.
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <fsm.h>

function control[CALC + 1];

int main (void)
{
	init_fsm(control);
	state st = INIT;
	while(1)
	{
		st = control[st]();
	}
}
