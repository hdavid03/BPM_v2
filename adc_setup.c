/*
 * adc_setup.c
 *
 * Created: 3/11/2022 4:13:42 PM
 *  Author: Hajdu Dávid
 */ 
#include "adc_setup.h"
#define CTRLA (ADC_DMASEL_OFF_gc | ADC_FLUSH_bm | ADC_ENABLE_bm)
#define CH_CTRL (ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_INTERNAL_gc)
#define CH_INTCTRL (ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc)

static struct adc_config conf = 
{
	.ctrla = CTRLA,
	.ctrlb = ADC_RESOLUTION_12BIT_gc,
	.refctrl = ADC_REFSEL_INTVCC_gc,
	.prescaler = ADC_PRESCALER_DIV512_gc
};

static struct adc_channel_config ch_conf =
{
	.ctrl = CH_CTRL,
	.muxctrl = ADC_CH_MUXINT_SCALEDVCC_gc,
	.intctrl = CH_INTCTRL
};

void adc_setup()
{
	// disable adc before writing the configuration
	adc_disable(&ADCA);
	
	// write configuration in to registers
	adc_write_configuration(&ADCA, &conf);
	adcch_write_configuration(&ADCA, ADC_CH0, &ch_conf);
}
