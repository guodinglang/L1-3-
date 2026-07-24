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

#ifndef  __COMMAND_H__
#define  __COMMAND_H__

#include "project.h"


//typedef enum {
//	succes = 0x00,	//命令已成功执行
//	FCE,	//帧格式错误
//	CE,		//校验错误
//	CPE,	//命令参数错误
//	EF,		//执行失败
//	ET,		//执行超时
//	MNR,	//模块未就绪
//	CMDE,	//命令错误
//	MWNCD 	//无校准数据的模块
//	
//} CMD_STS;



/*********************************************************************************/
int gLowerCaseString(char *str);
void rxCommand(void);
char help(void *p,...);
char ReadVersion(void *p,...);
char RebackCharacter(void *p,...);
char ReadWriteBoardIDN(void *p,...);
char ReadWriteBoardPN(void *p,...);
char ReadWriteBoardSN(void *p,...);
char ReadSelectSwitch(void *p,...);
char ReadWriteMaxCH(void *p,...);
char LevelValueDispose(void *p,...);
char OffsetChannel(void *p,...);
char SetReadWrite(void *p,...);


#endif


