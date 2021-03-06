/*
 * adc_setup.c
 *
 * Created: 3/11/2022 4:13:42 PM
 *  Author: Hajdu Dávid
 */ 
#include "adc_setup.h"
#define CTRLA (ADC_DMASEL_OFF_gc | ADC_FLUSH_bm | ADC_ENABLE_bm)
#define CH_CTRL (ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc)
#define CH_INTCTRL (ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc)

static struct adc_config conf = 
{
	.ctrla = CTRLA,
	.ctrlb = ADC_RESOLUTION_12BIT_gc | ADC_FREERUN_bm,
	.refctrl = ADC_REFSEL_INTVCC_gc,
	.prescaler = ADC_PRESCALER_DIV512_gc
};

static struct adc_channel_config ch_conf =
{
	.ctrl = CH_CTRL,
	.muxctrl = My_ADC_Input,
	.intctrl = CH_INTCTRL
};

union adc_buffer_element adc_buffer[ADC_BUFFSIZE];

void adc_setup()
{
	// disable adc before writing the configuration
	adc_disable(&ADCA);
	
	// write configuration in to registers
	adc_write_configuration(&ADCA, &conf);
	adcch_write_configuration(&ADCA, ADC_CH0, &ch_conf);
	adc_set_callback(&ADCA, &adc_read_result);
	fifo_init(&adc_fifo_desc, adc_buffer, ADC_BUFFSIZE);
}

uint8_t get_result(float* result)
{
	uint8_t retval = 0;
	uint16_t conv_result = 0;
	if(!fifo_is_empty(&adc_fifo_desc))
	{
		conv_result = fifo_pull_uint16_nocheck(&adc_fifo_desc);
		*result = convert_result_to_Hgmm(conv_result);
		retval = 1;
	}
	return retval;
}

void adc_read_result(ADC_t *adc, uint8_t ch_mask, adc_result_t res)
{
	if(!fifo_is_full(&adc_fifo_desc))
	{
		fifo_push_uint16_nocheck(&adc_fifo_desc, res);
	}
}


