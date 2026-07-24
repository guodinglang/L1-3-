


#include "SerialPort.h"

//***************************************************************************/
rxStructure   	urStruct1,*pUART=&urStruct1;

/***************************************************************************
;** 函数名称: 	usart_configuration
;** 功能描述:  	usart configuration
;** 输入参数: 
;** 返 回 值:   无
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2022-5-25
;** 修改原因：
;** 说    明:
;***************************************************************************/
void usart_configuration(uint32_t baud_rate)
{
  gpio_init_type gpio_init_struct;

  /* enable the usart1 and gpio clock */
  crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the usart1 tx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_9;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  /* configure the usart1 rx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_10;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(GPIOA, &gpio_init_struct);

  /* config usart nvic interrupt */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(USART1_IRQn, 0, 0);

  /* configure usart1 param */
  usart_init(USART1, baud_rate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(USART1, TRUE);
  usart_receiver_enable(USART1, TRUE);

  /* enable usart1 interrupt */
  usart_interrupt_enable(USART1, USART_RDBF_INT, TRUE);
//  usart_interrupt_enable(USART1, USART_TDBE_INT, TRUE);
  usart_enable(USART1, TRUE);

}


/***************************************************************************
;** 函数名称: 	
;** 功能描述:  USARTx 传送数据	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2012-6-29
;** 修改原因：
;** 说    明:
;***************************************************************************/
void printx(uint16_t ch)
{
	while(usart_flag_get(USART1, USART_TDBE_FLAG) == RESET);
	usart_data_transmit(USART1, ch);
}
/***************************************************************************
;** 函数名称: 	
;** 功能描述:  USARTx 传送一帧数据	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2012-6-29
;** 修改原因：
;** 说    明:
;***************************************************************************/
void printfx(char *st)
{
	while(*st!='\0')
	{
		while(usart_flag_get(USART1, USART_TDBE_FLAG) == RESET){;}
		usart_data_transmit(USART1, *st);
		st++;
	}
}

/***************************************************************************
;** 函数名称: 	
;** 功能描述:  USARTx printf	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2012-6-29
;** 修改原因：
;** 说    明:
;***************************************************************************/
void sprintfx(char *s, ...)
{
	char buffer[64]={NULL};   // 分配512字节的字符串缓存
    uint8_t len,i; 
	
    va_list ap;	
    va_start(ap, s);
    vsprintf(buffer, (const char *)s, ap);    // 将格式字符串与后面的参数绑定为一个字符串，拷贝到输出缓存
    va_end(ap);
		
 // 这里写自己的打印输出函数，比如这里用puts
//    puts(buffer);
	len=strlen(buffer);
	for(i=0;i<len;i++)
	{
		while(usart_flag_get(USART1, USART_TDBE_FLAG) == RESET);
		usart_data_transmit(USART1, buffer[i]);
	}
}

/***************************************************************************
;** 函数名称: 	
;** 功能描述:  USARTx 传送一帧数据	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-7-22
;** 修改原因：
;** 说    明:
;***************************************************************************/
void printfn(uint8_t *st,uint16_t num)
{
	uint16_t i;
	
	for(i=0;i<num;i++)
	{
		while(usart_flag_get(USART1, USART_TDBE_FLAG) == RESET){;}
		usart_data_transmit(USART1, st[i]);
	}
}









