
 /*
*********************************************************************************************************
*	                                  
*	模块名称 :     
*	文件名称 : 
*	版    本 : V1.0
*	说    明 : 头文件
*
*********************************************************************************************************
*/

#ifndef  __AD5504_H__
#define  __AD5504_H__



#include "project.h"

//--------------------------------------------------------------------------
#define  DAC_CH_A	0x1000
#define  DAC_CH_B	0x2000
#define  DAC_CH_C	0x3000
#define  DAC_CH_D	0x4000

#define  DAC_POWER_UP	0x703C	//C0 = 0: the device is not in thermal shutdown mode
								//C2/C5 = 1: DAC Channel A power-up  
								//C6 = 0: outputs connected to AGND through a 20K resistor
								
								
#define  DAC_POWER_DOWN 0x7000	//all channel power OFF


#define  DAC_POWER_REG  0xF000 //read


//==============================================================================
void SPI1_WriteAD5504(uint16_t *wd, uint8_t num);
void AD5504_PowerUp(uint8_t ch);
uint16_t SPI1_ReadAD5504(uint16_t reg);
void AD5504_PowerDown(uint8_t ch);






#endif

