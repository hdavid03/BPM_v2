/*
 * define_ports.h
 *
 * Created: 4/15/2022 10:35:45 PM
 *  Author: Hajdu Dávid
 */ 


#ifndef DEFINE_PORTS_H_
#define DEFINE_PORTS_H_

#include <asf.h>

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

//5V SW switch on the pressure sensor
#define PSWbm 0b01000000
#define PSWPORT PORTF
#define PSW_ON PSWPORT.OUTSET = PSWbm
#define PSW_OFF PSWPORT.OUTCLR = PSWbm

// drain valve
#define VALVEPORT PORTC
#define VALVEPINbm 0b00000100
#define VALVE_ON VALVEPORT.OUTSET = VALVEPINbm
#define VALVE_OFF VALVEPORT.OUTCLR = VALVEPINbm

#define PUSHBTN PORTE
#define PUSHBTNbm 0b00100000
#define BTNCHECK (PUSHBTN.IN & PUSHBTNbm)
#define INVERT_BTN_INPUT (PUSHBTN.PIN5CTRL |= (1 << 6))

#endif /* DEFINE_PORTS_H_ */