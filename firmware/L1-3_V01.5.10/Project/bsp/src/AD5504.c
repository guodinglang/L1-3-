

#include "AD5504.h"


/***************************************************************************
;** 函数名称:  	SPI_ReadWriteByte
;** 功能描述:   SPI读写N个字节（发送完成后返回本次通讯读取的数据）
;** 输入参数: 	uint8_t TxData 待发送的数
;** 返 回 值: 	无
;** 优 先 级:  
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:   2017-12-27
;** 修改原因：
;** 说    明:    
;***************************************************************************/ 
void SPI1_WriteAD5504(uint16_t *wd, uint8_t num)
{ 
	uint8_t i;
	uint32_t e;
	
	for(i=0;i<num;i++)
	{
		DAC_CS(1);	//20nS enable DAC
		delay_us(1);
		DAC_CS(0);
		
		e=1800000;	//100ms
		while(spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
		{
			if(e>0){e--;}
			else{ALARM(1);}
		}
		spi_i2s_data_transmit(SPI1, wd[i]);
		
		e=1800000;	//100ms
		while(spi_i2s_flag_get(SPI1, SPI_I2S_RDBF_FLAG) == RESET)
		{
			if(e>0){e--;}
			else{ALARM(1);}
		}
		e = spi_i2s_data_receive(SPI1);	
		delay_us(1);
	}

}

/***************************************************************************
;** 函数名称:  	
;** 功能描述:   SPI读字节（发送完成后返回本次通讯读取的数据）
;** 输入参数: 	
;** 返 回 值: 	
;** 优 先 级:  
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:   2017-12-27
;** 修改原因：
;** 说    明:    
;***************************************************************************/ 
uint16_t SPI1_ReadAD5504(uint16_t reg)
{
	uint32_t e;
	
	DAC_CS(1);	//20nS enable DAC
	delay_us(1);
	DAC_CS(0);
	
	e=1800000;	//100ms
	while(spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
	{
		if(e>0){e--;}
		else{ALARM(1);}
	}
	spi_i2s_data_transmit(SPI1, reg);
	
	e=1800000;	//100ms
	while(spi_i2s_flag_get(SPI1, SPI_I2S_RDBF_FLAG) == RESET)
	{
		if(e>0){e--;}
		else{ALARM(1);}
	}
	
	return  spi_i2s_data_receive(SPI1);		
}

/***************************************************************************
;** 函数名称:  	AD5504_PowerUp
;** 功能描述:   
;** 输入参数: 	ch  0xff-all on
;** 返 回 值: 	
;** 优 先 级:  
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:   2022-5-31
;** 修改原因：
;** 说    明:    
;***************************************************************************/ 
void AD5504_PowerUp(uint8_t ch)
{ 
	uint16_t d,tar[2]={0,0};
	
	d = SPI1_ReadAD5504(DAC_POWER_REG);	
	d &= 0x0ffe;
	d |= 0x7000;
	
	switch(ch)
	{
		case 0x00: break; 
		case 0x01:{tar[0] = d|0x0004;SPI1_WriteAD5504(tar,2);}break;	//ch a
		case 0x02:{tar[0] = d|0x0008;SPI1_WriteAD5504(tar,2);}break;
		case 0x03:{tar[0] = d|0x0010;SPI1_WriteAD5504(tar,2);}break;
		case 0x04:{tar[0] = d|0x0020;SPI1_WriteAD5504(tar,2);}break;
		case 0xff:{tar[0] = d|0x003C;SPI1_WriteAD5504(tar,2);}break;	//all on
		
		default:break;
	}
}
/***************************************************************************
;** 函数名称:  	AD5504_PowerUp
;** 功能描述:   
;** 输入参数: 	ch  0xff-all off
;** 返 回 值: 	
;** 优 先 级:  
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:   2022-5-31
;** 修改原因：
;** 说    明:    
;***************************************************************************/ 
void AD5504_PowerDown(uint8_t ch)
{ 
	uint16_t d,tar[2]={0,0};
	
	d = SPI1_ReadAD5504(DAC_POWER_REG);	
	d &= 0x0ffe;
	d |= 0x7000;
	switch(ch)
	{
		case 0x00: break; 
		case 0x01:{tar[0] = d&0xfffb;SPI1_WriteAD5504(tar,2);}break;	//ch a
		case 0x02:{tar[0] = d&0xfff7;SPI1_WriteAD5504(tar,2);}break;
		case 0x03:{tar[0] = d&0xffef;SPI1_WriteAD5504(tar,2);}break;
		case 0x04:{tar[0] = d&0xffdf;SPI1_WriteAD5504(tar,2);}break;
		case 0xff:{tar[0] = 0x7001;SPI1_WriteAD5504(tar,2);}break;	//all on	
		default:break;
	}
}










