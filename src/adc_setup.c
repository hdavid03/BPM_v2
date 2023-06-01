/*
 * adc_setup.c
 *
 * Created: 3/11/2022 4:13:42 PM
 *  Author: Hajdu Dávid
 */ 
#include <adc_setup.h>
#include <timer_setup.h>
#define My_ADC_Input 	(ADCCH_POS_PIN8 << ADC_CH_MUXPOS0_bp)
#define ADC_BUFFSIZE 	32U
#define ADC_CLOCK		31250UL
#define CTRLA 			(ADC_DMASEL_OFF_gc | ADC_FLUSH_bm | ADC_ENABLE_bm)
#define CH_CTRL 		(ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc)
#define CH_INTCTRL 		(ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc)

fifo_desc_t adc_fifo_desc;

static void evsys_setup(void);


static struct adc_config conf = 
{
	.ctrla = CTRLA,
	.prescaler = ADC_PRESCALER_DIV256_gc
};

static struct adc_channel_config ch_conf =
{
	.ctrl = CH_CTRL,
	.muxctrl = My_ADC_Input,
	.intctrl = CH_INTCTRL
};

union adc_buffer_element adc_buffer[ADC_BUFFSIZE];

static void evsys_setup(void)
{
	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_EVSYS);
	EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;
}

void adc_setup(void)
{
	// disable adc before writing the configuration
	adc_disable(&ADCA);
	evsys_setup();
	timer_setup();
	adc_set_conversion_trigger(&conf, ADC_TRIG_EVENT_SINGLE, 1, 0);
	adc_set_conversion_parameters(&conf, ADC_SIGN_OFF, ADC_RES_12, ADC_REF_VCC);
	adc_set_clock_rate(&conf, ADC_CLOCK);
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
		*result = convert_result_to_Hgmm((float)conv_result);
		retval = 1;
	}
	return retval;
}

void adc_read_result(ADC_t *adc, uint8_t ch_mask, adc_result_t res)
{
	if(!fifo_is_full(&adc_fifo_desc))
	{
		fifo_push_uint16_nocheck(&adc_fifo_desc, (uint16_t)res);
	}
}

void adc_reset_fifo(void)
{
	if (adc_is_enabled(&ADCA))
		adc_disable(&ADCA);
	while(!fifo_is_empty(&adc_fifo_desc))
	{
		fifo_pull_uint16_nocheck(&adc_fifo_desc);
	}
}

