

#include "Temperature.h"

//--------------------------------------------------------------------------
structTemp 	sTempere;


/***************************************************************************
;** 函数名称: 	M1601_IO_IN
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:  未使用 
;***************************************************************************/
void M1601_ConfigIO(void)
{
	gpio_init_type gpio_init_struct;
	
	crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
	
	/* configure PA Input*/
	gpio_init_struct.gpio_pins = GPIO_PINS_1;	//
	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init(GPIOA, &gpio_init_struct);
}

/***************************************************************************
;** 函数名称: 	M1601_IO_IN
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 
;***************************************************************************/
void M1601_IO_In(void)
{
	gpio_init_type gpio_init_struct;
	
	/* configure PA Input*/
	gpio_init_struct.gpio_pins = GPIO_PINS_1;	//
	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init(GPIOA, &gpio_init_struct);		
	
}

/***************************************************************************
;** 函数名称: 	M1601_IO_OUT
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 
;***************************************************************************/
void M1601_IO_Out(void)
{
	gpio_init_type gpio_init_struct;
	
	/* configure PA Output*/
	gpio_init_struct.gpio_pins = GPIO_PINS_1;		//
	gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init(GPIOA, &gpio_init_struct);		
	
}

/***************************************************************************
;** 函数名称: 	
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 
;***************************************************************************/
void M1601_RST(void)
{
	M1601_IO_Out();
	M1601_PO(0);
	delay_us(480);
	M1601_PO(1);
}

/***************************************************************************
;** 函数名称: 	
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因; 
;** 说    明:	 r =0 -- OK;  60-240us低电平信号构成的存在脉冲
;***************************************************************************/
int8_t M1601_ResetPresence(void)
{
	int8_t c;
	
	M1601_IO_Out();
	M1601_PO(0);
	delay_us(480);
	M1601_PO(1);
	M1601_IO_In(); 
	delay_us(70);	//After detecting the rising edge on the DQ pin, 
				    //the M601 waits for 15~60 us and then transmits 
	               //the presence pulse (a low signal for 60~240us).
	  			  // Get presence pulse from slave
	c=0;
	while((M1601_PI >0)&&(c <18))
	{
		delay_us(10);
		c++;
	}
	
	delay_us(400); 	// Complete the reset-presensce
	
	if(c <18) //小于180us is right
	{c=0;}
	
	return c; 
}

/***************************************************************************
;** 函数名称: 	M1601_ReadByte
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 r =0 -- OK; 
;***************************************************************************/
uint8_t M1601_ReadByte(void)
{
	uint8_t i,d=0;
	
	for(i=0;i<8;i++)
	{
		M1601_IO_Out(); // Initialte read time slot
		M1601_PO(0);
		delay_us(3);
		M1601_PO(1);
		M1601_IO_In();
		delay_us(10);
		d >>= 1;
		if(M1601_PI >0)
		{d |=0x80;}
		delay_us(55);
	}
	
	return d;	
}

/***************************************************************************
;** 函数名称: 	M1601_WriteByte
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 r =0 -- OK; 
;***************************************************************************/
void M1601_WriteByte(uint8_t d)
{
	uint8_t i;
	
	M1601_IO_Out();
	for(i=0;i<8;i++)
	{
		if(d&0x01)
		{// Write '1' to DQ 	//所有读时序必须最少60us
			M1601_PO(0);
			delay_us(3);
			M1601_PO(1);	
			delay_us(51);
		}
		else
		{// Write '0' to DQ
			M1601_PO(0);
			delay_us(54);
			M1601_PO(1); 
			delay_us(10);	//所有读时序必须最少60us
		}
		d >>=1;
	}
	
}

/***************************************************************************
;** 函数名称: 	M1601_Inital
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 r =0 -- OK; 
;***************************************************************************/
uint8_t M1601_Inital(void)
{
	M1601_ConfigIO();
	
	return M1601_ResetPresence();
}

/***************************************************************************
;** 函数名称: 	M1601_Inital
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 r =0 -- OK; 
;***************************************************************************/
float getTemperature(void)
{
	int16_t d;
	uint8_t i;
	float T=0.0;
	
	if(!M1601_ResetPresence())
	{
		M1601_WriteByte(0xCC);	//跳过ROM指令
		M1601_WriteByte(0x44);	//主控制器发温度转换指令
		delay_ms(12);
		if(!M1601_ResetPresence())
		{
			M1601_WriteByte(0xCC);	//跳过ROM指令
			M1601_WriteByte(0xBE);	//读寄存器指令
			
			i=M1601_ReadByte();	//LSB
			d=M1601_ReadByte();
			d=(d<<8)|i;
			
			T=((float)d)/256.0f + 40.0f;
		}
	}
	
	return T;
}

/***************************************************************************
;** 函数名称: 	M1601_Inital
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 r =0 -- OK; 
;***************************************************************************/
void Temperature(void)
{
	if(!sTempere.tOut)
	{
		switch(sTempere.step)
		{
			case 0:{
				if(!M1601_ResetPresence())
				{
					M1601_WriteByte(0xCC);	//跳过ROM指令
					M1601_WriteByte(0x44);	//主控制器发温度转换指令
					sTempere.tOut=15;		//等待时间
					sTempere.step++;
				}
			}break;
			case 1:{
				if(!M1601_ResetPresence())
				{
					int16_t i;
					
					M1601_WriteByte(0xCC);	//跳过ROM指令
					M1601_WriteByte(0xBE);	//读寄存器指令
					
					i = M1601_ReadByte();	//LSB
					i |=(M1601_ReadByte()<<8);
					
					if((i != 0)||(i != 0xff))
					{
						sTempere.tValue = ((float)i)/256.0f + 40.0f;
					}
					
//					if((sTempere.tValue < -30.0f)||(sTempere.tValue >70.0f))
//					{
//						erCode.bit.b0=true;
//						ALARM(1);
//					}
//					else if(erCode.bit.b0)
//					{
//						erCode.bit.b0=false;
//						ALARM(0);
//					}
					sTempere.step=0;
					sTempere.tOut=1200;	//1ms*x
				}
			}break;
			default:sTempere.step=0;break;
		}
	}
}

/***************************************************************************
;** 函数名称: 	M1601_Inital
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 r =0 -- OK; 
;***************************************************************************/
void MCP9700Config(void)
{
//	gpio_init_type gpio_init_struct;
//	/* enable dac/gpioa clock */
//	crm_periph_clock_enable(CRM_DAC_PERIPH_CLOCK, TRUE);
//	crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
//	
//	gpio_init_struct.gpio_pins = GPIO_PINS_1;
//	gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
//	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
//	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
//	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
//	gpio_init(GPIOA, &gpio_init_struct);
//	
//	/* dac1 configuration */
//	dac_trigger_select(DAC1_SELECT, DAC_SOFTWARE_TRIGGER);
//	dac_trigger_enable(DAC1_SELECT, TRUE);
//	dac_wave_generate(DAC1_SELECT, DAC_WAVE_GENERATE_NONE);
//	dac_mask_amplitude_select(DAC1_SELECT, DAC_LSFR_BITB0_AMPLITUDE_4095);
//	dac_output_buffer_enable(DAC1_SELECT, FALSE);	
//	dac_enable(DAC1_SELECT, TRUE);

}

/***************************************************************************
;** 函数名称: 	M1601_Inital
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2024-4-16
;** 修改原因;
;** 说    明:	 VOUT = TC x TA + V0°C ; V0°C=0.5V; TC=10mV/°C
;***************************************************************************/
void AcquisitMCP9700A(void)
{
	if(!sTempere.tOut)
	{
		if(sTempere.idx <100)
		{
			sTempere.tadc +=ADC1_GetSingle(ADC_CHANNEL_1);
			sTempere.idx++;
		}
		else
		{
			sTempere.tValue = 3.3f/4096*(sTempere.tadc/sTempere.idx);
			sTempere.tValue -= 0.5f;
			sTempere.tValue = sTempere.tValue/0.01f + 2;
			sTempere.idx = 0;
			sTempere.tadc = 0;
			sTempere.tOut = 150;	//10MS*X 1.5s
		}
	}
	
}


