
/****************************************************************************************
*
*	ģ������ : BSP(Board Support Package)
*	�ļ����� : bsp.c
*	��    �� : V1.0
*	˵    �� : BSP �弶֧�ְ�(Board Support Package)
*	�޸ļ�¼ :
*   �� �� �� :  
*   ��    �� :     
*   ��    �� :   
*	˵    �� :
*
*****************************************************************************************/
#include "IOstream.h"

//--------------------------------------------------------------------------
BitStructure16    erCode; //��¼�������
//ButtonManage  	  button,*pkey=&button;
uint16_t keycn[2]={0,0};

/***************************************************************************
;** ��������:  IoConfig
;** ��������:  
;** �������:
;** �� �� ֵ:
;** ȫ�ֱ���: 
;** ����ģ��: 
;** ����  ��:   
;** �ա�  ��:  2022-2-13
;** �޸�ԭ��
;** ˵    ��:  MCU GPIO
;***************************************************************************/
void IoConfig(void)
{
	gpio_init_type gpio_init_struct;
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE); /* enable peri clk */
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE); /* enable peri clk */
    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE); /* enable peri clk */

    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;  
    gpio_init_struct.gpio_pins           = GPIO_PINS_13;  // led
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(GPIOC, &gpio_init_struct);

    gpio_init_struct.gpio_pins           = GPIO_PINS_7; // M_SPREAD
    gpio_init(GPIOA, &gpio_init_struct);

    gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins           = GPIO_PINS_12; // M_ORG
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull           = GPIO_PULL_UP;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(GPIOA, &gpio_init_struct);
        
	/* 2. M_DIAG (PA11) - ��ϺͶ�ת״̬��� - ���룬���� */
    gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins           = GPIO_PINS_8|GPIO_PINS_11; // PA8 M_INDEX  PA11 M_DIAG
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull           = GPIO_PULL_UP;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(GPIOA, &gpio_init_struct);
    
    gpio_init_struct.gpio_pins           = DIN_0_PIN|DIN_1_PIN|DIN_4_PIN|DIN_5_PIN;
    gpio_init(DIN_0_PORT, &gpio_init_struct);
    
    gpio_init_struct.gpio_pins           = DIN_2_PIN|DIN_3_PIN;
    gpio_init(GPIOA, &gpio_init_struct);
    
    gpio_init_struct.gpio_pins           = STORE_PIN; //����
    gpio_init(STORE_PORT, &gpio_init_struct);
    
    gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_pins           = ALARM_PIN;   //���
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(ALARM_PORT, &gpio_init_struct);
    
    gpio_init_struct.gpio_pins           = BUSY_PIN; //���  
    gpio_init(BUSY_PORT, &gpio_init_struct);
	
	BUSY(ON);		//output high
	S_LED(ON);
}

/***************************************************************************
;** ��������:  keyConfig
;** ��������:  
;** �������:  ��
;** �� �� ֵ: 
;** ȫ�ֱ���: 
;** ����ģ��: 
;** ����  ��:   
;** �ա�  ��:  2022-2-13
;** �޸�ԭ��
;** ˵    ��:
;***************************************************************************/
void exint_line15_config(void)
{
  exint_init_type exint_init_struct;
	
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
  
  gpio_exint_line_config(GPIO_PORT_SOURCE_GPIOB, GPIO_PINS_SOURCE5);

  exint_default_para_init(&exint_init_struct);
  exint_init_struct.line_enable = TRUE;
  exint_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
  exint_init_struct.line_select = EXINT_LINE_5;
  exint_init_struct.line_polarity = EXINT_TRIGGER_FALLING_EDGE;
  exint_init(&exint_init_struct);

  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(EXINT9_5_IRQn, 1, 0);
	
}

/***************************************************************************
;** ��������:  	InputIOScan
;** ��������:  	
;** �������:
;** �� �� ֵ: 	
;** �� �� ����  
;** ȫ�ֱ���: 
;** ����ģ��: 
;** ����  ��:   G-D-L
;** �ա�  ��:   2026-6-11
;** �޸�ԭ��  AT32F403A ������Ŀ
;***************************************************************************/
void InputIOScan(void)
{	
	uint16_t m=0;
	
	m = (0x0330 & gpio_input_data_read(GPIOB));	
	m |= ((0x60 & gpio_input_data_read(GPIOA))>>4);
	
	if(m != swMess.buff)
	{
		if(swMess.valid)
		{
			if(keycn[0]>0)
			{keycn[0]--;}
			else
			{
				swMess.buff = m;
				swMess.valid = false;
			}
		}
		else
		{
			swMess.buff = m;
			keycn[0]=0;
		}
	}	
	else
	{		
		if(keycn[0]<10)//12000)//3.35us*x
		{keycn[0]++;}
		else
		{//1.62us*12000=19440us
			if(swMess.valid == false)
			{				
				swMess.parallel = true;
				swMess.valid = true;
				BUSY(1);
			}
		}
	}
		
	
//	if(!DAC_ALARM)
//	{
//		if(keycn[1] < 10000)
//		{
//			keycn[1]++;
//			if(keycn[1]==9998)
//			{ALARM(1);}
//		}	
//	}
//	else
//	{
//		if(keycn[1] > 0)
//		{
//			keycn[1]--;
//			if(keycn[1]==1)
//			{ALARM(0);}
//		}
//	}

}

