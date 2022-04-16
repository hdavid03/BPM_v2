/*
 * fsm.c
 *
 * Created: 4/15/2022 5:33:02 PM
 *  Author: Hajdu Dávid
 */ 
#include "fsm.h"
#include "clk_setup.h"
#include "adc_setup.h"
#include "pwm_setup.h"
#include "usartf0.h"
#include <string.h>
#include <stdio.h>

static function control[CALC + 1];

void init_fsm(function* control)
{
	control[INIT] = init_bpm;
	control[IDLE] = check_button;
	control[DC_ON] = dc_on;
	control[PUMP] = check_pressure;
	control[DC_OFF] = dc_off;
	control[CALC] = calculation;
}

state init_bpm()
{
	setup_48MHz_12MHz_clock();
	TCC0_setup();
	LED1PORT.DIRSET = LED1bm;	//set output
	LED2PORT.DIRSET = LED2bm;
	LED1_OFF;
	LED2_OFF;
	PSWPORT.DIRSET = PSWbm;
	PSW_ON;
	MOTOR_OFF;
	MOTORPORT.DIRSET  = MOTORbm;
	PUSHBTN.DIRCLR = PUSHBTNbm;
	INVERT_BTN_INPUT;
	usartf0_init();
	pmic_enable_level(PMIC_LVL_LOW);									//Proc Multylevel Interrupt Controller (PMIC) enable IT level LOW
	adc_setup();
	adc_enable(&ADCA);
	sei();
	return IDLE;
}

state check_button()
{
	if(BTNCHECK)
		return DC_ON;
	return IDLE;
}

state dc_on()
{
	set_pwm(10000);
	MOTOR_ON;
	return PUMP;
}

state check_pressure()
{
	uint16_t result;
	do{}while(!get_result(&result));
	uint16_t resHgmm = (uint16_t)(code_to_Hgmm * result + 0.5);
	char str[16] = "";
	sprintf(str, "P: %u\n", resHgmm);
	usart_putstring(str);
	if(resHgmm >= 230)
		return DC_OFF;
	return PUMP;
}

state dc_off()
{
	MOTOR_OFF;
	set_pwm(9000);
	return CALC;
}

state calculation()
{
	uint16_t result;
	do{}while(!get_result(&result));
	uint16_t resHgmm = (uint16_t)(code_to_Hgmm * result + 0.5);
	char str[16] = "";
	sprintf(str, "P: %u\n", resHgmm);
	usart_putstring(str);
	if(resHgmm <= 45)
	{
		set_pwm(0);
		return IDLE;
	}
	return CALC;
}