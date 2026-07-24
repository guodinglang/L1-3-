

#ifndef   __TEMPERATURE__H__
#define   __TEMPERATURE__H__

#include "project.h"

//-----------------------------------------------------------------------------------------------
#define M1601_PO(n)       n>0? (GPIOA->scr = GPIO_PINS_1):(GPIOA->clr = GPIO_PINS_1) 
#define M1601_PI		 (GPIO_PINS_1 & GPIOA->idt)



//-----------------------------------------------------------------------------------------------
typedef struct{
	uint8_t 	step;	//
	uint16_t	tOut;	//timeout
	uint32_t    tadc;	//
	uint8_t     idx;	//下标计数
	float		tValue;	//Temperatrue value;
	
}structTemp;

extern structTemp 	sTempere;
//-----------------------------------------------------------------------------------------------
uint8_t M1601_Inital(void);
float getTemperature(void);
void Temperature(void);
void M1601_ConfigIO(void);
void AcquisitMCP9700A(void);





#endif



