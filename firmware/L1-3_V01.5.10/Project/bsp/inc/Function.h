

#ifndef  __FUNCTION_H_
#define  __FUNCTION_H_
  
#include "project.h"  

//--------------------------------------------------------------------------
#define SLED     (GPIOD->odt ^= GPIO_PINS_1)


//--------------------------------------------------------------------------
typedef struct{
	bool      parallel;	//并行输入控制	
	bool      valid;	//有效
	uint8_t   ch;		//选择的通道
	uint16_t  buff;		//D0--D5数据保存
	
}SwitchStruct;

extern SwitchStruct  swMess;
//--------------------------------------------------------------------------
//typedef struct{
////	bool      state;
////	bool      upload;	
//	uint8_t   step;
//	uint8_t   digit;
//	uint16_t  setv;
//	int16_t   oadc;
//	
//}calibrationStruct;

//extern calibrationStruct calib;
//--------------------------------------------------------------------------
void WorkStateLED(void);
void EmpowerCheck(void);
void SwitchMessage(void);



#endif




