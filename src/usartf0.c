#include <usartf0.h>
#include <string.h>

#define USART_INT_LVL USART_INT_LVL_LO

#define UART0PORT PORTF
#define UARTF0_TXbm 0b00001000
#define UARTF0_RXbm 0b00000100

#define USART_SERIAL                     &USARTF0
#define USART_SERIAL_BAUDRATE            19200
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false

#define RX_BUFF_SIZE 16
#define TX_BUFF_SIZE 32

fifo_desc_t rxfifo_desc, txfifo_desc;

static usart_serial_options_t usart_options = {
	.baudrate = USART_SERIAL_BAUDRATE,
	.charlength = USART_SERIAL_CHAR_LENGTH,
	.paritytype = USART_SERIAL_PARITY,
	.stopbits = USART_SERIAL_STOP_BIT
};

static volatile unsigned int transmit_stop;
union rxbuffer_element {
	uint8_t  byte;
	uint16_t halfword;
	uint32_t word;
};

union rxbuffer_element fifo_rxbuf[RX_BUFF_SIZE];

union txbuffer_element {
	uint8_t  byte;
	uint16_t halfword;
	uint32_t word;
};

union txbuffer_element fifo_txbuf[TX_BUFF_SIZE];

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
	usart_set_rx_interrupt_level(USART_SERIAL, USART_INT_LVL);		//rx IT level LO
	usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL_OFF);		//tx IT disable
	
	fifo_init(&rxfifo_desc, fifo_rxbuf, RX_BUFF_SIZE);					//Initialize the rx FIFO
	fifo_init(&txfifo_desc, fifo_txbuf, TX_BUFF_SIZE);					//Initialize the tx FIFO
	transmit_stop = 1;	
}

uint8_t get_char(char *chr)
{
	uint8_t retval = 0;
	if(!fifo_is_empty(&rxfifo_desc))
	{
		*chr = fifo_pull_uint8_nocheck(&rxfifo_desc);
		retval = 1;
	}
	return retval;
}

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
	usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL);
}

void usart_putbytes(const char bytes[], uint8_t size)
{
	while(!fifo_is_empty(&txfifo_desc)){}
	for(uint8_t i = 0; i < size; i++)
	{
		if(!fifo_is_full(&txfifo_desc))
		{
			fifo_push_uint8(&txfifo_desc, bytes[i]);
		}
		else
		{
			break;
		}
	}
	usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL);
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

char bin_hex(char binb)
{
	char hex_chr;
	binb &= 0x0f;
	if (binb <= 0x09)
		hex_chr = binb + '0';
	else
		hex_chr = binb + 'A' - 10;
	return hex_chr;
}