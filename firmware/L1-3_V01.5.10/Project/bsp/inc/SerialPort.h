#ifndef   __SERIALPORT_H__
#define  __SERIALPORT_H__

#include "project.h"

//#include "stdint.h"
//#include "stm32f10x.h"

//========================================================================
#define	RX_MAX_FIFO     128
//#define	RESTR           32



//========================================================================
typedef struct{
	bool                feelback;
	unsigned char       rxBuf[RX_MAX_FIFO];
	unsigned char       len;     //接收总长度
	unsigned char       rxn;     //单次接收个数	
	unsigned char       reTx; 	 //返回字符位置
	uint16_t            TimeOut; //超时
	
}rxStructure;

extern rxStructure   *pUART;
//========================================================================





//========================================================================
void usart_configuration(uint32_t baud_rate);
void printx(uint16_t ch);
void printfx(char *st);
void sprintfx(char *s, ...);
void printfn(uint8_t *st,uint16_t num);



#endif




