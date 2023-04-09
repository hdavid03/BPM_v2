/*
 * usartf0.h
 *
 * Created: 2022.02.27. 13:02:10
 *  Author: Zoltan
 */ 

#ifndef USARTF0_H_
#define USARTF0_H_

#include <asf.h>

void usartf0_init(void);
void usart_putstring(const char str[]);
void usart_putbytes(const char bytes[], uint8_t size);
uint8_t get_char(char *chr);
char bin_hex(char binb);

#endif /* USARTF0_H_ */