//#include "delay.h"

#include "chokeDelay.h"

static __IO uint32_t fac_us;
static __IO uint32_t fac_ms;

/***************************************************************************
;** 函数名称: 	
;** 功能描述:  	us delay
;** 输入参数:   无
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2012-6-29
;** 修改原因：
;** 说    明:	 不是很准确	   <=255us
;***************************************************************************/
void delay_us(uint32_t nus)
{
  uint32_t temp = 0;
  SysTick->LOAD = (uint32_t)(nus * fac_us);
  SysTick->VAL = 0x00;
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;
  do
  {
    temp = SysTick->CTRL;
  }while((temp & 0x01) && !(temp & (1 << 16)));

  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
  SysTick->VAL = 0x00;
}
/***************************************************************************
;** 函数名称: 	
;** 功能描述:  	us delay
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:   2012-6-29
;** 修改原因：
;** 说    明:	用systick获得准确的ms延时 
;***************************************************************************/
void delay_ms( uint16_t ms)
{
	uint32_t start_time = system_tick;

    while((system_tick - start_time) < ms)
    {
        __nop();
    }
}
/***************************************************************************
;** 函数名称: 	delay_init
;** 功能描述:  	
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-11
;** 修改原因：
;** 说    明:	用systic获得us ms延时 
;***************************************************************************/
void delay_init()
{
  /* configure systick */
  systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
  fac_us = system_core_clock / (1000000U);
  fac_ms = fac_us * (1000U);
}


