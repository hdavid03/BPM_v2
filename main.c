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
#include <asf.h>
#include "clk_setup.h"
#include "adc_setup.h"
#include <avr/delay.h>
#include "usartf0.h"
#include <string.h>
#include <stdio.h>


//LED1 E1
//LED2 E0
#define LED1PORT PORTE
#define LED1bm 0b00000010
#define LED2PORT PORTE
#define LED2bm 0b00000001
#define LED1_OFF LED1PORT.OUTCLR = LED1bm;
#define LED1_ON LED1PORT.OUTSET = LED1bm;
#define LED2_OFF LED2PORT.OUTCLR = LED2bm;
#define LED2_ON LED2PORT.OUTSET = LED2bm;

#define MOTORPORT PORTD
#define MOTORbm 0b00000100
#define MOTOR_ON MOTORPORT.OUTSET = MOTORbm
#define MOTOR_OFF MOTORPORT.OUTCLR  = MOTORbm
#define OFF 0
#define ON 1

uint8_t M = 0;

//5V SW switch on the pressure sensor
#define PSWbm 0b01000000
#define PSWPORT PORTF
#define PSW_ON PSWPORT.OUTSET = PSWbm
#define PSW_OFF PSWPORT.OUTCLR = PSWbm

#define OC0PORT PORTC
#define OC0PINbm 0b00000100


//CTRLA:  – – – – CLKSEL[3:0]
#define DIV4 0x03
//CTRLB: CCDEN CCCEN CCBEN CCAEN – WGMODE[2:0]
//         0     1     1    0    0
#define CCCEN 6
#define CCBEN 5
#define DSTOP 0x05
#define WGMODE DSTOP
#define PWM_PERIOD (30000-1) // T = PWMPERIOD*Tclk   (Tclk = 1/12MHz)
#define pwm_max ((PWM_PERIOD + 1)/3 - 1)

uint16_t PWM_TH = 0;    // THpwm = PWM_Time*Tclk
#define set_pwm(x) {if(3*x > PWM_PERIOD) {tc_write_cc(&TCC0, TC_CCC, PWM_PERIOD);} else {tc_write_cc(&TCC0, TC_CCC, 3*x);}}  //0 <= pwm <=10000

void TCC0_Setup(void);
void TCC0_Setup(void)
{
	TCC0.CTRLA = DIV4;	//prescaler 4, fclk = 12MHz/4 = 3 MHz
	TCC0.CTRLB = (1<<CCCEN)|(1<<CCBEN)|WGMODE;  //OC0C and OC0B enabled, and dual slope PWM with OVF on TOP
	tc_enable(&TCC0);
	tc_set_wgm(&TCC0, TC_WG_SS);
	tc_write_period(&TCC0, PWM_PERIOD);				//PWM fequency: 3e6/2*(3e4) = 50 Hz
	tc_write_cc(&TCC0, TC_CCC, PWM_TH);			// duty cycle: PWM_TH/PWM_PERIOD
	tc_enable_cc_channels(&TCC0,TC_CCCEN);
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV4_gc);
	OC0PORT.DIRSET = OC0PINbm;					//OC0PORT set as output
	set_pwm(10000);
}

void setup(void);
void setup(void)
{
	setup_48MHz_12MHz_clock();
	TCC0_Setup();
	LED1PORT.DIRSET = LED1bm;	//set output
	LED2PORT.DIRSET = LED2bm;
	LED1_OFF;
	LED2_OFF;	
	PSWPORT.DIRSET = PSWbm;
	PSW_ON;
	MOTOR_OFF;
	M = OFF;
	MOTORPORT.DIRSET  = MOTORbm;
	usartf0_init();
	pmic_enable_level(PMIC_LVL_LOW);									//Proc Multylevel Interrupt Controller (PMIC) enable IT level LOW
	adc_setup();
	adc_enable(&ADCA);
	sei();																// global IT enable
}

#define divR 0.571
#define code_to_V (float)(3.3/(divR*1.6*4095))
#define V_to_kPa (float)(50/4.5)
#define kPa_to_Hgmm 7.50062


int main (void)
{
	char str[48];
	char tmpstr[2];
	char result_string[8];
	tmpstr[1] = 0;
	static uint16_t offset, result, pwm, step, resHgmm;
	static volatile float resfl, offsetfl, resflV;
	pwm = 10000;
	step = 100;
	offset = 0;
	setup();
	
	for (int i = 0; i < 8; i++)
	{
		do{}while(!get_result(&result));
		offset += result;
	}
	
	offset = offset / 8;
	offsetfl = (float)offset;
	
	while(1)
	{
		// adc_start_conversion(&ADCA, ADC_CH0);
		// adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
		
		/* Insert system clock initialization code here (sysclk_init()). */
		// result = adc_get_result(&ADCA, ADC_CH0);
		// sprintf(result_string, " %u", result);
		/*
		_delay_ms(250);
		LED1_ON;
		LED2_ON;
		_delay_ms(250);
		LED1_OFF;
		LED2_OFF;
		*/
/*
// 		strcpy(str, "a ");
// 		
// 		if(get_result(result_string))
// 		{
// 			strcat(str, "Pressure: ");
// 			strcat(str, result_string);
// 		}
// 		
// 		if(get_char(&tmpstr[0]))
// 		{
// 			strcpy(str, " received char: ");
// 			strcat(str, tmpstr);
// 		}
// 		
// 		strcat(str,"\r\n");
// 		usart_putstring(str);
// 		//_delay_ms(100);
// 		PWM_TH = (PWM_TH + 1)%PWM_PERIOD;
// 		set_pwm(PWM_TH);
*/
		if(get_char(&tmpstr[0]))
		{
			strcat(str,tmpstr);
			strcat(str,"\r\n");
			usart_putstring(str);
			switch(tmpstr[0])
			{
				case 'm':
				if(M == OFF)
				{
					MOTOR_ON;
					M = ON;
					usart_putstring("M: ON");
				}
				else
				{
					MOTOR_OFF;
					M = OFF;
					usart_putstring("M: OFF");
				}
				break;
				case 'f':
				if(pwm <= pwm_max)
				{
					pwm = pwm + step;
					set_pwm(pwm);
					sprintf(str, "pwm: %i \r\n", pwm);
					usart_putstring(str);
				}
				break;
				case 'v':
				if(pwm >= step)
				{
					pwm = pwm - step;
					set_pwm(pwm);
					sprintf(str, "pwm: %i \r\n", pwm);
					usart_putstring(str);
				}
				break;
				case 'q':
				MOTOR_OFF;
				set_pwm(0);
				usart_putstring("All OFF!");
				break;
				default:
				usart_putstring("Command error!");
				break;
			}
		}
		else
		{
			do{}while(!get_result(&result));
			
			sprintf(str, "%i code ", result);
			usart_putstring(str);
			
			result = result-offset;
			if(result < 0) result = 0;
			resfl = (float)result;
			
			resflV = code_to_V*(resfl + offsetfl);
			sprintf(str, "%.2f V ", resflV);  //to use sprintf float: include stdio.h, project/properties/AVR/GNU Linker/General:check use vprintf,
			//""   Miscellaneous/Other Linker Flags: -lprintf_flt
			usart_putstring(str);

			resfl = code_to_V*(resfl);
			sprintf(str, "%.2f V-Voff ", resfl);
			usart_putstring(str);
			
			resfl = V_to_kPa*resfl;
			sprintf(str, "%.2f kPa ", resfl);
			usart_putstring(str);
			
			resfl = kPa_to_Hgmm*resfl;
			sprintf(str, "%.2f Hgmmfl ", resfl);
			usart_putstring(str);
			
			resHgmm = (int16_t)(resfl + 0.5);
			
			sprintf(str, "%i Hgmm ", resHgmm);
			usart_putstring(str);
			sprintf(str, "%i pwm\r\n", pwm);
			usart_putstring(str);
			
			if(resHgmm >= 240)
			{
				MOTOR_OFF;
				M = OFF;
				usart_putstring("M: OFF, over pressure!");
			}
		}
	}
	/* Insert application code here, after the board has been initialized. */
}
