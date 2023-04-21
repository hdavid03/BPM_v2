/*
 * fsm.c
 *
 * Created: 4/15/2022 5:33:02 PM
 *  Author: Hajdu Dávid
 */ 
#include <util/delay.h>
#include <string.h>
#include "fsm.h"
#include "clk_setup.h"
#include "adc_setup.h"
#include "pwm_setup.h"
#include "usartf0.h"

static void status_ok_led(void);
static void init_ports(void);

static void status_ok_led(void)
{
	LED1_ON;
	LED2_ON;
	_delay_ms(1000);
	LED1_OFF;
	LED2_OFF;
}

static void init_ports(void)
{
	LED1PORT.DIRSET = LED1bm;	//set output
	LED2PORT.DIRSET = LED2bm;
	MOTORPORT.DIRSET  = MOTORbm;
	PSWPORT.DIRSET = PSWbm;
	VALVEPORT.DIRSET = VALVEPINbm;
	PUSHBTN.DIRCLR = PUSHBTNbm;
	INVERT_BTN_INPUT;
	LED1_OFF;
	LED2_OFF;
	VALVE_OFF;
	PSW_ON;
	MOTOR_OFF;
}

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
	init_ports();
	usartf0_init();
	pmic_enable_level(PMIC_LVL_LOW);	//Proc Multylevel Interrupt Controller (PMIC) enable IT level LOW
	adc_setup();
	status_ok_led();
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
	complete_conversion(&(resHgmm.value));
	usart_putbytes(resHgmm.bytes, sizeof(float));
	if(resHgmm.value >= 100.0f)
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
	complete_conversion(&(resHgmm.value));
	usart_putbytes(resHgmm.bytes, sizeof(float));
	if(resHgmm.value <= 40.0f)
	{
		usart_putstring("XXXX");
		adc_disable(&ADCA);
		VALVE_OFF;
		LED1_OFF;
		PSW_OFF;
		return IDLE;
	}
	return CALC;
}