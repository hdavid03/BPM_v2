/*
 * fsm.h
 *
 * Created: 4/15/2022 5:33:21 PM
 *  Author: Hajdu Dávid
 */ 


#ifndef FSM_H_
#define FSM_H_

#include "define_ports.h"

typedef enum { INIT, IDLE, DC_ON, PUMP, DC_OFF, CALC, END } state;
typedef state (*function)(void);

state init_peripherals(void);
state check_button(void);
state dc_on(void);
state check_pressure(void);
state dc_off(void);
state calculation(void);
state result(void);

void init_fsm(function*);

#endif /* FSM_H_ */