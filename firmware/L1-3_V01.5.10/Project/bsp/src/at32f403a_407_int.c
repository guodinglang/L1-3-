/**
  **************************************************************************
  * @file     at32f403a_407_int.c
  * @brief    main interrupt service routines.
  **************************************************************************
  *
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

/* includes ------------------------------------------------------------------*/
#include "project.h"

/** @addtogroup AT32F403A_periph_examples
  * @{
  */

/** @addtogroup 403A_SPI_w25q_flash
  * @{
  */

/**
  * @brief  this function handles nmi exception.
  * @param  none
  * @retval none
  */
void NMI_Handler(void)
{
}

/**
  * @brief  this function handles hard fault exception.
  * @param  none
  * @retval none
  */
void HardFault_Handler(void)
{
  /* go to infinite loop when hard fault exception occurs */
  while(1)
  {
//	  ALR_LED(1);
	  ALARM(1);
  }
}

/**
  * @brief  this function handles memory manage exception.
  * @param  none
  * @retval none
  */
void MemManage_Handler(void)
{
  /* go to infinite loop when memory manage exception occurs */
  while(1)
  {
//	  ALR_LED(1);
	  ALARM(1);
  }
}

/**
  * @brief  this function handles bus fault exception.
  * @param  none
  * @retval none
  */
void BusFault_Handler(void)
{
  /* go to infinite loop when bus fault exception occurs */
  while(1)
  {
//	  ALR_LED(1);
	  ALARM(1);
  }
}

/**
  * @brief  this function handles usage fault exception.
  * @param  none
  * @retval none
  */
void UsageFault_Handler(void)
{
  /* go to infinite loop when usage fault exception occurs */
  while(1)
  {
//	  ALR_LED(1);
	  ALARM(1);
  }
}

/**
  * @brief  this function handles svcall exception.
  * @param  none
  * @retval none
  */
void SVC_Handler(void)
{
}

/**
  * @brief  this function handles debug monitor exception.
  * @param  none
  * @retval none
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  this function handles pendsv_handler exception.
  * @param  none
  * @retval none
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  this function handles systick handler.
  * @param  none
  * @retval none
  */
void SysTick_Handler(void)
{
    static uint32_t ticks = 0;
    system_tick++;
    ticks ++;

    if(pUART->TimeOut > 0)
    {
		pUART->TimeOut--;
		if(pUART->TimeOut == 0)
		{
			memset(pUART->rxBuf,0,RX_MAX_FIFO);
			pUART->len = 0;
			pUART->rxn = 0;
		}
    }
    /* toggle led */
    if(ticks >= DELAY)
    {
		ticks = 0;
    }
	
	if(wait_for_compensation_cnt > 1)
	{
		wait_for_compensation_cnt--;
//		gpio_bits_toggle(STORE_PORT, STORE_PIN);
	}
	
//	if((system_tick % 1000) == 0)
//	{
//		int32_t cnt = Encoder_AB_GetCount();
//        sprintfx("enc=%ld\r\n", cnt);
//		sprintfx("loc=%d\r\n", motor_status.position);
//	}
}

/**
  * @}
  */

void USART1_IRQHandler()
{
//    uint8_t rcvData;
//    if(usart_flag_get(USART1, USART_RDBF_FLAG) != RESET)
//    {
//        rcvData = usart_data_receive(USART1);
//        pUART->TimeOut = 100;    //1ms*x

//        if(rcvData == '>')
//        {
//            pUART->rxn = 1;
//        }
//        else if((pUART->len < RX_MAX_FIFO)&&(!pUART->rxn))
//        {
//            pUART->rxFIFO[pUART->len++] = rcvData;
//        }
//    }
    uint8_t s;
	if(usart_flag_get(USART1, USART_RDBF_FLAG) != RESET)
	{
		s = USART1->dt;
		pUART->TimeOut=60;	//10ms*x
		
		if((s=='\r')||(s=='\n'))
        {pUART->rxn++;}
		else if(pUART->len < RX_MAX_FIFO)
		{pUART->rxBuf[pUART->len++]=s;}
		
		usart_flag_clear(USART1, USART_RDBF_FLAG);
		
	}
}

bool g_origin_location_flag;
void EXINT3_IRQHandler(void)
{
	if(exint_flag_get(EXINT_LINE_3) != RESET)
	{
		exint_flag_clear(EXINT_LINE_3);
		
//		pin_states.org_state = 1;
//		g_origin_location_flag = 1;
	}
}

void EXINT9_5_IRQHandler(void)
{
	if(exint_flag_get(EXINT_LINE_5) != RESET)
	{
		exint_flag_clear(EXINT_LINE_5);
	}
}

/**
  * @brief  exint15 interrupt handler
  * @param  none
  * @retval none
  */
void EXINT15_10_IRQHandler(void)
{
    if (exint_flag_get(EXINT_LINE_11) != RESET) 
    {
        // ����жϱ�־
        exint_flag_clear(EXINT_LINE_11); // M_DIAG �������״̬�ж�
        
        // ��ת��⴦��
        tmc2209_stall_detected();
    }
	else if (exint_flag_get(EXINT_LINE_12) != RESET) 
	{
		exint_flag_clear(EXINT_LINE_12);
		g_origin_location_flag = 1;
	} 
}



/**
  * @}
  */



