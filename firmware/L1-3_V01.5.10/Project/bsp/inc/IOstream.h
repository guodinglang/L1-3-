 /*
*********************************************************************************************************
*	                                  
*	ģ������ :     
*	�ļ����� : 
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*********************************************************************************************************
*/
#ifndef __IOSTREAM_H__
#define	__IOSTREAM_H__

#include "project.h"

//--------------------------------------------------------------------------
/**************** define print uart ******************/
#define PRINT_UART                       USART1
#define PRINT_UART_CRM_CLK               CRM_USART1_PERIPH_CLOCK
#define PRINT_UART_TX_PIN                GPIO_PINS_9
#define PRINT_UART_TX_GPIO               GPIOA
#define PRINT_UART_TX_GPIO_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK


#define LED_PORT    (GPIOC)
#define LED_PIN     (GPIO_PINS_13)

#define DIN_0_PORT  (GPIOB)
#define DIN_0_PIN   (GPIO_PINS_5)
#define DIN_1_PORT  (GPIOB)
#define DIN_1_PIN   (GPIO_PINS_8)
#define DIN_2_PORT  (GPIOA)
#define DIN_2_PIN   (GPIO_PINS_6)
#define DIN_3_PORT  (GPIOA)
#define DIN_3_PIN   (GPIO_PINS_5)
#define DIN_4_PORT  (GPIOB)
#define DIN_4_PIN   (GPIO_PINS_9)
#define DIN_5_PORT  (GPIOB)
#define DIN_5_PIN   (GPIO_PINS_4)
#define DIN_6_PORT  (GPIOB)
#define DIN_6_PIN   (GPIO_PINS_7)
#define DIN_7_PORT  (GPIOB)
#define DIN_7_PIN   (GPIO_PINS_6)

#define NTC1601_PORT    (GPIOA)
#define NTC1601_PIN     (GPIO_PINS_1)

#define STORE_PORT      (GPIOA)
#define STORE_PIN       (GPIO_PINS_2)

#define ALARM_PORT      (GPIOA)
#define ALARM_PIN       (GPIO_PINS_3)

#define BUSY_PORT       (GPIOA)
#define BUSY_PIN        (GPIO_PINS_4)
/**
  * @}
  */
#define S_LED(n)         n>0? (GPIOC->scr = GPIO_PINS_13):(GPIOC->clr = GPIO_PINS_13)// S_LED 
#define DAC_R_SET(n)     n>0? (GPIOA->scr = GPIO_PINS_3):(GPIOA->clr = GPIO_PINS_3)	 // 0=60V;1=30V
#define DAC_CLR(n)       n>0? (GPIOB->scr = GPIO_PINS_1):(GPIOB->clr = GPIO_PINS_1)	 // �½�������Ĵ������������Ϊ0V;����ΪH
#define DAC_CS(n)        n>0? (GPIOB->scr = GPIO_PINS_0):(GPIOB->clr = GPIO_PINS_0)	 //
#define DAC_LDAC(n)      n>0? (GPIOA->scr = GPIO_PINS_4):(GPIOA->clr = GPIO_PINS_4)  // 0=�����Ĵ����仯���˿ڣ�1=���ֶ˿ڵ�ѹ��
#define BOOST_FSET(n)    n>0? (GPIOA->scr = GPIO_PINS_8):(GPIOA->clr = GPIO_PINS_8)	 // 1=1.1MHZ;0=650KHZ
#define BOOST_SHDN(n)    n>0? (GPIOA->scr = GPIO_PINS_11):(GPIOA->clr = GPIO_PINS_11)// 1=ON;0=OFF

#define BUSY(n)          n>0? (GPIOA->scr = GPIO_PINS_4):(GPIOA->clr = GPIO_PINS_4)
#define ALARM(n)         n>0? (GPIOA->scr = GPIO_PINS_3):(GPIOA->clr = GPIO_PINS_3)

#define STROBE           (GPIO_PINS_2 & GPIOA->idt)
#define DAC_ALARM        (GPIO_PINS_3 & GPIOA->idt)
#define EP_MODE          (GPIO_PINS_2 & GPIOA->idt)

typedef union 
{
	unsigned short int regis;	//16 bit
	struct 
	{
		unsigned short int b0:1;
		unsigned short int b1:1;
		unsigned short int b2:1;
		unsigned short int b3:1;
		unsigned short int b4:1;
		unsigned short int b5:1;
		unsigned short int b6:1;
		unsigned short int b7:1;	
		unsigned short int b8:1;
		unsigned short int b9:1;
		unsigned short int b10:1;
		unsigned short int b11:1;
		unsigned short int b12:1;
		unsigned short int b13:1;
		unsigned short int b14:1;
		unsigned short int b15:1;		
	}bit;
}BitStructure16;

extern BitStructure16  erCode;
//-------------------------------------------------------------------------------
void IoConfig(void);
void BatteryConfig(void);
void InputIOScan(void);
//void SwitchCallback(void);
void exint_line15_config(void);





#endif




