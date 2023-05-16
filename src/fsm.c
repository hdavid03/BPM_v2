/*
 * fsm.c
 *
 * Created: 4/15/2022 5:33:02 PM
 *  Author: Hajdu Dávid
 */ 
#include <util/delay.h>
#include <string.h>
#include <fsm.h>
#include <clk_setup.h>
#include <adc_setup.h>
#include <usartf0.h>
#include <peak_filter.h>

#define UART_MARKER "XXXX"
#define PUMP_STOP	190.0f
#define LET_DOWN	50.0f

static void status_ok_led(void);
static void init_ports(void);

static peak_filter_t peak_filter;

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
	MOTOR_OFF;
}

void init_fsm(function* control)
{
	control[INIT]	= init_peripherals;
	control[IDLE]	= check_button;
	control[DC_ON]	= dc_on;
	control[PUMP]	= check_pressure;
	control[DC_OFF]	= dc_off;
	control[CALC]	= calculation;
}

state init_peripherals(void)
{
	setup_clock();
	init_ports();
	usartf0_init();
	adc_setup();
	pmic_enable_level(PMIC_LVL_LOW);	//Proc Multylevel Interrupt Controller (PMIC) enable IT level LOW
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
	PSW_ON;
	VALVE_ON;
	_delay_ms(100);
	MOTOR_ON;
	LED2_ON;
	peak_filter_init(&peak_filter);
	adc_enable(&ADCA);
	return PUMP;
}

state check_pressure(void)
{
	float filter_output = 0.0f;
	float_byteblock resHgmm;
	complete_conversion(&(resHgmm.value));
	filter_output = (float)filter_sample(&peak_filter, resHgmm.value);
	usart_putbytes((const char*)&filter_output, sizeof(float));
	if(resHgmm.value > PUMP_STOP)
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
	float filter_output = 0.0f;
	float_byteblock resHgmm;
	complete_conversion(&(resHgmm.value));
	filter_output = (float)filter_sample(&peak_filter, resHgmm.value);
	usart_putbytes((const char*)&filter_output, sizeof(float));
	if(resHgmm.value <= LET_DOWN)
	{
		adc_flush(&ADCA);
		adc_disable(&ADCA);
		usart_putstring(UART_MARKER);
		_delay_ms(200);
		VALVE_OFF;
		LED1_OFF;
		PSW_OFF;
		return IDLE;
	}
	return CALC;
}