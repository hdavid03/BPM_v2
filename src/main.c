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
#include <asf.h>

#define F_CPU 12000000
#include <avr/delay.h>
#include <string.h>

//LED1 E1
//LED2 E0
#define LED1PORT PORTE
#define LED1bm 0b00000010
#define LED2PORT PORTE
#define LED2bm 0b00000001

#define UART0PORT PORTF
#define UARTF0_TXbm 0b00001000
#define UARTF0_RXbm 0b00000100

#define USART_SERIAL                     &USARTF0
#define USART_SERIAL_BAUDRATE            57600
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false
	
static usart_serial_options_t usart_options = {
	.baudrate = USART_SERIAL_BAUDRATE,
	.charlength = USART_SERIAL_CHAR_LENGTH,
	.paritytype = USART_SERIAL_PARITY,
	.stopbits = USART_SERIAL_STOP_BIT
};

#define rxbuf_size 16
#define txbuf_size 32

static volatile unsigned int transmit_stop;
union rxbuffer_element {
	uint8_t  byte;
	uint16_t halfword;
	uint32_t word;
};

union rxbuffer_element fifo_rxbuf[rxbuf_size];
fifo_desc_t rxfifo_desc;

union txbuffer_element {
	uint8_t  byte;
	uint16_t halfword;
	uint32_t word;
};

union txbuffer_element fifo_txbuf[txbuf_size];
fifo_desc_t txfifo_desc;

void usartf0_init(void);
void usartf0_init(void)
{
	UART0PORT.DIRSET = UARTF0_TXbm; //set output
	UART0PORT.DIRCLR = UARTF0_RXbm; //set input

	usart_serial_init(USART_SERIAL, &usart_options);
	/*
	USART_INT_LVL_OFF
	USART_INT_LVL_LO
	USART_INT_LVL_MED
	USART_INT_LVL_HI
	*/
	usart_set_rx_interrupt_level(USART_SERIAL, USART_INT_LVL_HI);		//rx IT level LO
	usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL_OFF);		//tx IT disable
	//usart_set_tx_interrupt_level(USART_SERIAL, USART_INT_LVL_LO);	
	fifo_init(&rxfifo_desc, fifo_rxbuf, rxbuf_size);					//Initialize the rx FIFO
	fifo_init(&txfifo_desc, fifo_txbuf, txbuf_size);					//Initialize the tx FIFO
	transmit_stop = 1;	
}

static volatile char chr;
void usart_putstring(const char str[]);
void usart_putstring(const char str[])
{	
	while(!fifo_is_empty(&txfifo_desc)){}	
	for(int i = 0; i < strlen(str); i++)
	{
		if(!fifo_is_full(&txfifo_desc))
		{							
			fifo_push_uint8(&txfifo_desc, str[i]);
		}
		else
		{
			break;
		}			
	}
	usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL_HI);
}

//set proper values on conf_clock.h

//#define CONFIG_PLL0_SOURCE          PLL_SRC_XOSC
//#define CONFIG_PLL0_MUL             3
//#define CONFIG_PLL0_DIV             1

// External oscillator frequency range
#define CONFIG_XOSC_RANGE XOSC_RANGE_12TO16

// USB clock
#define CONFIG_USBCLK_SOURCE                USBCLK_SRC_PLL

// CPU clock, 16MHz*3/(2*2) = 12 MHz
//#define CONFIG_SYSCLK_SOURCE				SYSCLK_SRC_XOSC
#define CONFIG_SYSCLK_PSADIV				SYSCLK_PSADIV_2
#define CONFIG_SYSCLK_PSBCDIV				SYSCLK_PSBCDIV_1_2

void setup_48MHz_12MHz_clock(void);
void setup_48MHz_12MHz_clock(void)
{
	CCP=0xD8;    // Enable the clock source
	OSC.XOSCCTRL = XOSC_RANGE_12TO16|7; // select frequ range for 16MHz external XOSC

	CCP=0xD8;    // Enable the clock source
	OSC.CTRL = OSC_XOSCEN_bm; // enable XOSC enable
	while ((OSC.STATUS & OSC_XOSCRDY_bm)==0) {} // wait for it to become stable

	OSC.PLLCTRL = PLL_SRC_XOSC | CONFIG_PLL0_MUL ; // 0x20 = PLLDIV (PPLL input: XOSC, PLLDIV: 3: -> frequ = 3*16 = 48 MHz)

	OSC.CTRL |= OSC_PLLEN_bm ; // enable the PLL...
	while( (OSC.STATUS & OSC_PLLRDY_bm) == 0 ){} // wait until it's stable

	CCP=0xD8;
	CLK.CTRL = CLK_SCLKSEL_PLL_gc;	// The System clock source is now  PLL (48 MHz/PSADIV_1, )
	
	if ((CONFIG_SYSCLK_PSADIV != SYSCLK_PSADIV_1)
	|| (CONFIG_SYSCLK_PSBCDIV != SYSCLK_PSBCDIV_1_1)) {
		//			CLK.PSCTRL
		sysclk_set_prescalers(CONFIG_SYSCLK_PSADIV,CONFIG_SYSCLK_PSBCDIV);
	}
}



//USART0 transmit IT (data register empty)
ISR(USARTF0_DRE_vect)
{
	if(!fifo_is_empty(&txfifo_desc))
	{
		usart_putchar(USART_SERIAL, fifo_pull_uint8_nocheck(&txfifo_desc));		
	}
	else
	{
		usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL_OFF);	//if FIFO empty, disable transmit IT
	}
}

//USART0 receive IT ()
ISR(USARTF0_RXC_vect)
{
	unsigned char datin;
	datin = usart_get(USART_SERIAL);
	if(!fifo_is_full(&rxfifo_desc))
	{
		fifo_push_uint8(&rxfifo_desc, datin);		
	}
}

void setup(void);
void setup(void)
{
	setup_48MHz_12MHz_clock();
	LED1PORT.DIRSET = LED1bm;	//set output
	LED2PORT.DIRSET = LED2bm;
	LED1PORT.OUTCLR = LED1bm;
	LED2PORT.OUTCLR = LED2bm;	
	usartf0_init();
	pmic_enable_level(PMIC_LVL_HIGH);									//Proc Multylevel Interrupt Controller (PMIC) enable IT level LOW
	sei();																// global IT enable
}

int main (void)
{
	char str[32];
	char tmpstr[2];
    tmpstr[1] = 0;		
	/* Insert system clock initialization code here (sysclk_init()). */
	setup();
	do{
		LED1PORT.OUTSET = LED1bm;
		LED2PORT.OUTSET = LED2bm;
		_delay_ms(250);
		LED1PORT.OUTCLR = LED1bm;
		LED2PORT.OUTCLR = LED2bm;
		_delay_ms(250);	

		if(!fifo_is_empty(&rxfifo_desc))
		{
			tmpstr[0] = fifo_pull_uint8_nocheck(&rxfifo_desc);
			strcpy(str,"received char: ");
			strcat(str,tmpstr);
			strcat(str,"\r\n");
			usart_putstring(str);		
		}
				
	}while(1);

	/* Insert application code here, after the board has been initialized. */
}
