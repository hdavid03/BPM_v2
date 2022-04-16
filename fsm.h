/*
 * fsm.h
 *
 * Created: 4/15/2022 5:33:21 PM
 *  Author: Hajdu Dávid
 */ 


#ifndef FSM_H_
#define FSM_H_

#include "define_ports.h"

#define divR 0.571
#define code_to_V (float)(3.3 / (divR * 1.6 * 4095))
#define V_to_Hgmm (float)(50 / 4.5 * 7.50062)
#define code_to_Hgmm code_to_V * V_to_Hgmm

typedef enum { INIT, IDLE, DC_ON, PUMP, DC_OFF, CALC } state;
typedef state (*function)(void);

state init_bpm(void);
state check_button(void);
state dc_on(void);
state check_pressure(void);
state dc_off(void);
state calculation(void);

void init_fsm(function*);

#endif /* FSM_H_ */