

//#include <pulicinclude.h>	/* 该文件包含了必需的.h文件 */
#include "InteriorAD.h"

//============================================================================


/***************************************************************************
;** 函数名称:  InterADconfig
;** 功能描述:  
;** 输入参数:  无
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   
;** 日　  期:  2017-10-18
;** 修改原因：
;** 说    明:
;***************************************************************************/
void InterADconfig(void)
{
	gpio_init_type gpio_initstructure;
	adc_base_config_type adc_base_struct;
	
	crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
	crm_periph_clock_enable(CRM_ADC1_PERIPH_CLOCK, TRUE);

	
	gpio_default_para_init(&gpio_initstructure);
	gpio_initstructure.gpio_mode = GPIO_MODE_ANALOG;
	gpio_initstructure.gpio_pins = GPIO_PINS_1;
	gpio_init(GPIOA, &gpio_initstructure);
	
		
//---------------------------------------------------------------------	
  adc_base_default_para_init(&adc_base_struct);
  adc_base_struct.sequence_mode = TRUE;
  adc_base_struct.repeat_mode = TRUE;
  adc_base_struct.data_align = ADC_RIGHT_ALIGNMENT;
  adc_base_struct.ordinary_channel_length = 1;//3;
  adc_base_config(ADC1, &adc_base_struct);
  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_1, 1, ADC_SAMPLETIME_239_5);
//  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_5, 2, ADC_SAMPLETIME_239_5);
//  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_6, 3, ADC_SAMPLETIME_239_5);
  adc_ordinary_conversion_trigger_set(ADC1, ADC12_ORDINARY_TRIG_SOFTWARE, TRUE);
  adc_dma_mode_enable(ADC1, TRUE);

  adc_enable(ADC1, TRUE);
  adc_calibration_init(ADC1);
  while(adc_calibration_init_status_get(ADC1));
  adc_calibration_start(ADC1);
  while(adc_calibration_status_get(ADC1));

}

/***************************************************************************
;** 函数名称:  
;** 功能描述:  这个函数通过设置ADC1单次采样单通道ADC_Channel，并将值返回
;** 输入参数:  无
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   
;** 日　  期:  2017-10-18
;** 修改原因：
;** 说    明:			
*               
;***************************************************************************/
uint16_t ADC1_GetSingle(adc_channel_select_type ADC_CH)
{
	adc_ordinary_channel_set(ADC1, ADC_CH, 1, ADC_SAMPLETIME_239_5);
	adc_ordinary_software_trigger_enable(ADC1, TRUE);
	while(!adc_flag_get(ADC1,ADC_CCE_FLAG)){;}
		
	return adc_ordinary_conversion_data_get(ADC1);
}



