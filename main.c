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
#define CONVMODE_SIGNED (1 << 4)
#define CONVMODE_UNSIGNED (0 << 4)
#define DIV512 (7 << 0)
#define REF_INTVCC (1 << 4)
#define FREERUN (1 << 3)
#define NOT_FREERUN (0 << 3)
#define RES_12BIT_RIGHT (0 << 1)
#define RES_8BIT (2 << 1)
#define RES_12BIT_LEFT (3 << 1)
#define GAIN_1X (0 << 2)
#define GAIN_2X (1 << 2)
#define GAIN_4X (2 << 2)
#define GAIN_8X (3 << 2)
#define INTERNAL_INPUT (0 << 0)
#define SINGLEENDED (1 << 0)
#define TEMP_INPUT (0 << 3)
#define BANDGAP_INPUT (1 << 3)
#define SCALEDVCC_INPUT (2 << 3)
#define DAC_INPUT (3 << 3)
#define CONV_COMPLETE (0 << 2)
#define COMP_BELOW (1 << 2)
#define COMP_ABOVE (3 << 2)
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
uint16_t PWM_TH = 0;    // THpwm = PWM_Time*Tclk
#define set_pwm(pwm) {if(pwm > PWM_PERIOD) {pwm = PWM_PERIOD;} else {tc_write_cc(&TCC0, TC_CCC, pwm);}}

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
	usartf0_init();
	pmic_enable_level(PMIC_LVL_LOW);									//Proc Multylevel Interrupt Controller (PMIC) enable IT level LOW
	sei();																// global IT enable
}


int main (void)
{
	char str[32];
	char tmpstr[2];
	tmpstr[1] = 0;
	register16_t result;
	
	setup();
	adc_setup();
	adc_enable(&ADCA);
	
	while(1)
	{
		ADCA.CTRLA |= (ADC_CH0 << ADC_CH0START_bp);

		adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
		/* Insert system clock initialization code here (sysclk_init()). */
		result = adc_get_result(&ADCA, ADC_CH0);
		
		/*
		_delay_ms(250);
		LED1_ON;
		LED2_ON;
		_delay_ms(250);
		LED1_OFF;
		LED2_OFF;
		*/
		
		
		if(get_char(&tmpstr[0]))
		{
			strcpy(str,"received char: ");
			strcat(str,tmpstr);
			strcat(str,"\r\n");
			usart_putstring(str);
		}
		

		PWM_TH = (PWM_TH + 1)%PWM_PERIOD;
		set_pwm(PWM_TH);
		
	}
	
	
	/* Insert application code here, after the board has been initialized. */
}
