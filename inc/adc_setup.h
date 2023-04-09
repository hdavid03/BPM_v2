/*
 * adc_setup.h
 *
 * Created: 3/11/2022 4:13:19 PM
 *  Author: Hajdu Dávid
 */ 


#ifndef ADC_SETUP_H_
#define ADC_SETUP_H_

#include <asf.h>

#define div_R 0.571
#define code_to_V (float)(3.3 / (div_R * 1.6 * 4096))

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
static inline float convert_result_to_Hgmm(float x) { return (x * code_to_V / 5 - 0.04) / 0.018 * 7.50062; }
static inline void complete_conversion(float_byteblock* byteblock) { do {} while(!get_result(&byteblock->value)); }

#endif /* ADC_SETUP_H_ */