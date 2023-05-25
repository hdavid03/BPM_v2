/*
 * adc_setup.h
 *
 * Created: 3/11/2022 4:13:19 PM
 *  Author: Hajdu Dávid
 */ 


#ifndef ADC_SETUP_H_
#define ADC_SETUP_H_

#include <asf.h>

#define DIVR 0.571
#define VREF 2.0625
#define VOFFSET 0.103125
#define ADC_TOP 4096
#define code_to_V(res) ((res * VREF / ADC_TOP - VOFFSET) / DIVR)

union adc_buffer_element
{
	uint8_t  byte;
	uint16_t halfword;
	uint32_t word;
};

typedef union
{
	float value;
	char bytes[sizeof(float)];
} float_byteblock;

void adc_setup(void);
void adc_read_result(ADC_t*, uint8_t, adc_result_t);
uint8_t get_result(float*);
void adc_reset_fifo(void);

static inline float convert_result_to_Hgmm(float x)
{
	return (code_to_V(x) / 5 - 0.04) / 0.018 * 7.50062;
}

static inline void complete_conversion(float* res)
{
	do {} while(!get_result(res));
}

#endif /* ADC_SETUP_H_ */