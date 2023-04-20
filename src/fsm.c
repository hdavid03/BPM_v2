﻿/*
 * fsm.c
 *
 * Created: 4/15/2022 5:33:02 PM
 *  Author: Hajdu Dávid
 */ 
#include <string.h>
#include "fsm.h"
#include "clk_setup.h"
#include "adc_setup.h"
#include "pwm_setup.h"
#include "usartf0.h"

void init_fsm(function* control)
{
	control[INIT]	= init_bpm;
	control[IDLE]	= check_button;
	control[DC_ON]	= dc_on;
	control[PUMP]	= check_pressure;
	control[DC_OFF]	= dc_off;
	control[CALC]	= calculation;
}

state init_bpm(void)
{
	setup_48MHz_12MHz_clock();
	LED1PORT.DIRSET = LED1bm;	//set output
	LED2PORT.DIRSET = LED2bm;
	MOTORPORT.DIRSET  = MOTORbm;
	PSWPORT.DIRSET = PSWbm;
	VALVEPORT.DIRSET = VALVEPINbm;
	LED1_OFF;
	LED2_OFF;
	PSW_ON;
	MOTOR_OFF;
	PUSHBTN.DIRCLR = PUSHBTNbm;
	INVERT_BTN_INPUT;
	usartf0_init();
	pmic_enable_level(PMIC_LVL_LOW);	//Proc Multylevel Interrupt Controller (PMIC) enable IT level LOW
	adc_setup();
	sei();
	return IDLE;
}

state check_button(void)
{
	if(BTNCHECK)
		return DC_ON;
	return IDLE;
}

state dc_on(void)
{
	VALVE_ON;
	MOTOR_ON;
	LED2_ON;
	adc_enable(&ADCA);
	return PUMP;
}

state check_pressure(void)
{
	float_byteblock resHgmm;
	complete_conversion(&resHgmm);
	usart_putbytes(resHgmm.bytes, sizeof(float));
	if(resHgmm.value >= 220.0f)
		return DC_OFF;
	return PUMP;
}

state dc_off(void)
{
	MOTOR_OFF;
	LED2_OFF;
	LED1_ON;
	return CALC;
}

state calculation(void)
{
	float_byteblock resHgmm;
	complete_conversion(&resHgmm);
	usart_putbytes(resHgmm.bytes, sizeof(float));
	if(resHgmm.value <= 50.0f)
	{
		VALVE_OFF;
		LED1_OFF;
		adc_disable(&ADCA);
		usart_putstring("XXXX");
		PSW_OFF;
		return IDLE;
	}
	return CALC;
}