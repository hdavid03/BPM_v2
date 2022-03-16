/*
 * usartf0.h
 *
 * Created: 2022.02.27. 13:02:10
 *  Author: Zoltan
 */ 


#ifndef USARTF0_H_
#define USARTF0_H_

#include <asf.h>

#define USART_INT_LVL USART_INT_LVL_LO

#define UART0PORT PORTF
#define UARTF0_TXbm 0b00001000
#define UARTF0_RXbm 0b00000100

#define USART_SERIAL                     &USARTF0
#define USART_SERIAL_BAUDRATE            57600
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false

#define rxbuf_size 16
#define txbuf_size 32

fifo_desc_t rxfifo_desc, txfifo_desc;

void usartf0_init(void);
void usart_putstring(const char str[]);
uint8_t get_char(char *chr);


#endif /* USARTF0_H_ */