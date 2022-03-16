﻿/*
 * adc_setup.h
 *
 * Created: 3/11/2022 4:13:19 PM
 *  Author: Hajdu Dávid
 */ 


#ifndef ADC_SETUP_H_
#define ADC_SETUP_H_

#include <asf.h>

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

void adc_setup(void);




#endif /* ADC_SETUP_H_ */