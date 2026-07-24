
/****************************************************************************************
*
*	模块名称 : BSP(Board Support Package)
*	文件名称 : 
*	版    本 : V0.1
*	说    明 : BSP 板级支持包(Board Support Package)
*	修改记录 :
*   版 本 号 :  
*   日    期 :     
*   作    者 :   
*	说    明 :
*
*****************************************************************************************/
#include "project.h"
#include "Initial.h"

//--------------------------------------------------------------------------
ParameterSturct  params;
LevelSturct  sLevel;//[LEVEL_NUM+1];	//


/***************************************************************************
;** 函数名称: 	SysTickConfigure
;** 功能描述:   SysTick Time Configure
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:   2022-5-26
;** 修改原因：  
;** 说    明:  	设置10ms依次systick，做温度采集和RX通信延时用
;***************************************************************************/
void SysTickConfigure(void)
{
  /* configure systick */
	systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
//  SysTick->LOAD = (uint32_t)(system_core_clock / (1000U));
//  SysTick->VAL = 0x00;
//  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;
	SysTick_Config(system_core_clock / 1000U); //1MS
}

/***************************************************************************
;** 函数名称: 	configWDT
;** 功能描述:   config WDT
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2022-5-26
;** 修改原因:
;** 说    明:  	
;***************************************************************************/
void configWDT(uint16_t d)
{
	/* reset from wdt */
	crm_flag_clear(CRM_WDT_RESET_FLAG);
	/* disable register write protection */
	wdt_register_write_enable(TRUE);

	/* set the wdt divider value */
	wdt_divider_set(WDT_CLK_DIV_32);	//0.8ms*d

	/* set reload value

	timeout = reload_value * (divider / lick_freq )    (s)

	lick_freq    = 40000 Hz
	divider      = 64		//1.6ms*d
	reload_value = d		//12bit=4095

	timeout = d * (64 / 40000 ) = max:6.552s = 6552ms
	*/
	wdt_reload_value_set(d - 1);

	/* reload wdt counter */
	wdt_counter_reload();

	/* enable wdt */
	wdt_enable();
	
}

/***************************************************************************
;** 函数名称:   W25Q128_ParametInit
;** 功能描述:   取出W25Q128 flash 存储的配置信息
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-11
;** 修改原因：
;** 说    明:  	W25Q128 16M byte
;***************************************************************************/
void W25Q128_ParametInit(void)
{
	uint8_t i;
	
	W25Q128_ReadMulByte(FLASH_SECTION_PARAMETER,&params.fInitial,1);
	if(0xAE != params.fInitial)
	{	
		params.fInitial=0xAE;	//标记初始化
		
		
		memset(params.IDN,0,sizeof(params.IDN));
		memcpy(params.IDN,"MOTOR_VER1.1",12);
		memset(params.PN,0,17);
		memcpy(params.PN,"PN:123456789",12);
		memset(params.SN,0,sizeof(params.SN));
		memcpy(params.SN,"SN:0123456789ABC",16);
		memset(params.ver,0,sizeof(params.ver));
		memcpy(params.ver,"PCB:L1-2,PRG:V0.2",17);
		
		params.nCH = 16;	
		params.pps=0xFF;
		
		for(i=0;i<4;i++)
		{
			params.k[i]=0;
			params.ol[i]=0;
		}	//保留4位小数点

		W25Q128_SaveParamet(FLASH_SECTION_PARAMETER,(uint8_t*)&params);
	}
	
	W25Q128_ReadParamet(FLASH_SECTION_PARAMETER,(uint8_t*)&params);
}

/***************************************************************************
;** 函数名称:   Time3_Init
;** 功能描述:   配置M_STEP的定时器3，用于驱动步进电机
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-11
;** 修改原因：
;** 说    明:
;***************************************************************************/
void Time3_Init(void)
{
    gpio_init_type gpio_init_struct;
//    tmr_output_config_type tmr_oc_init_structure;
    crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK, TRUE); /* enable tmr3 clock */
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE); /* enable peri clk */

    /* gpioa configuration:tmr3 channel3 as alternate function push-pull */
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_pins     = TMC_STEP_PIN;  // M_STEP
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull     = GPIO_PULL_NONE;
    gpio_init_struct.gpio_mode     = GPIO_MODE_MUX;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(TMC_STEP_PORT, &gpio_init_struct);

    tmr_base_init(TMR3, 65535, 120 - 1);
    tmr_cnt_dir_set(TMR3, TMR_COUNT_UP);
    tmr_clock_source_div_set(TMR3, TMR_CLOCK_DIV1);
}

/***************************************************************************
;** 函数名称: 
;** 功能描述:  硬件的初始化
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2013-3-17
;** 修改原因:
;** 说    明:  
;***************************************************************************/
void bsp_Init(void)
{
	swMess.parallel = false;
	pUART->feelback = false;
	memset(pUART->rxBuf,0,RX_MAX_FIFO);
	pUART->len = 0;
	pUART->rxn = 0;
	memset(&swMess,0,sizeof(swMess));
	M_DIAG_exti_config();
	D_EZ_exti_config();
	Encoder_AB_Init();
//	AD5504_PowerUp(0xff);	//all channel on
//	AD5504_PowerDown(0xff);	//all off
	delay_ms(100);
	swMess.ch = 0;
	BUSY(0);	//initial ok
	
	printfx(" Initial OK\r\n");	
}


/***************************************************************************
;** 函数名称: 
;** 功能描述:  浮点型格式化小数点 带四舍五入
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:  2013-3-17
;** 修改原因：
;** 说    明:  	 leng 保几位小数点
;***************************************************************************/
double ffp(double fm, int leng)
{
	double  ft=0.0;
    unsigned long ul=0;

	if(fabs(fm)>0.0)
	{
		if(leng>=0)
		{
			if(leng>=5)	 //只允许保留5位小数点
			{leng=5;}
			leng++;
			ul=(unsigned long)(fabs(fm)*pow(10,leng));	  //10的leng次方
			if((ul%10)>=5)
			{ul=ul/10+1;}	  //四舍五入
			else
			{ul=ul/10;}
			ft=(double)(ul/pow(10,leng-1)); //保留的小数点
			if(fm<0.0)
			{ft=-ft;}
		}
		else 
		{ft=fm*pow(10,leng);}	  //负次方
	}
	return ft;
}

/***************************************************************************
** 函数名称:  	
** 功能描述:  	转换浮点数n 为字符串str。
** 输入参数: 	 
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2013-4-25
** 修改原因：
** 说    明:    字符串表示为科学计数法形式，由decimals指定四舍五入保留小数位（最多五位）。
;***************************************************************************/
char ftoa(double fv,unsigned char decimals,char *str)
{
	double fd;
	unsigned char i,n,c,k;
	unsigned int v,pw;
	
	c=0;
	if(fv<0.0)
	{str[c++]='-';}
	fv=fabs(fv);
	v=fv;
	fd=fv-v;
	v=fd*pow(10,decimals+1); //四舍五入
	if((v%10)>=5)
	{fv=fv+1/pow(10,decimals);}		
	v=fv;k=0;
	for(i=0;i<10;i++)	//0xffffffff=4 294 967 295
	{
		pw=pow(10,9-i);
		n=v/pw;
		v=v%pw;
		if((n>0)||(k>0))
		{str[c+k]=n+'0';k++;}
	}	
	c=c+k;
	if(k<=0)
	{str[c++]='0';}	
	if(decimals>0)
	{
		str[c++]='.';
		v=fv;
		fd=fv-v;
		for(i=0;i<decimals;i++)
		{
			fd=fd*10;
			v=fd;
			n=v%10;
			str[c++]=n+'0';
		}	
	}
	return c;

}
/***************************************************************************
** 函数名称:  	char itoa(int tv,char *str)
** 功能描述:  	整数 为字符串string。
** 输入参数:    tv--需要转换的整型；
** 返 回 值: 	c--返回个数； *str--返回字符串
** 全局变量: 	
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2019-9-13
** 修改原因：
** 说    明:    
			0xffffffff = 4 294 967 295;
;***************************************************************************/
char itoa(int tv,char *str)
{
	unsigned char i,n,c;
	unsigned int v;
	
	v=abs(tv);
	for(i=0;i<10;i++)	//0xffffffff=4 294 967 295
	{
		str[i]=(v%10)+'0';
		v=v/10;
		if(v<=0)
		{break;}
	}
	c=i+1;	
	if(tv<0)
	{str[c]='-';c++;}	
	for(i=0;i<(c/2);i++)
	{//高低位互换
		n=str[i];
		str[i]=str[c-i-1];
		str[c-i-1]=n;
	}	
	
	return c;
}

/***************************************************************************
;** 函数名称:   M_DIAG_exti_config
;** 功能描述:   TMC2209 DIAG错误信号引脚初始化
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-11
;** 修改原因：
;** 说    明:  	M_DIAG 马达错误信息输入口，使用外部中断处理
;***************************************************************************/
void M_DIAG_exti_config(void) 
{
    gpio_init_type gpio_init_struct;
    // 配置外部中断
    exint_init_type exti_init_struct;
    // 启用GPIOA时钟
	crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    
    // 配置PA11为上拉输入
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_pins = M_DIAG_PIN;  // M_DIAG
	gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(M_DIAG_PORT, &gpio_init_struct);
    // 选择PA11作为EXTI11
    gpio_exint_line_config(GPIO_PORT_SOURCE_GPIOA, GPIO_PINS_SOURCE11);
    
    exint_default_para_init(&exti_init_struct);
    exti_init_struct.line_enable = TRUE;
    exti_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
    exti_init_struct.line_select = EXINT_LINE_11;
    exti_init_struct.line_polarity = EXINT_TRIGGER_RISING_EDGE;
    exint_init(&exti_init_struct);
    
    // 配置NVIC
//    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(EXINT15_10_IRQn, 1, 3);
}

/***************************************************************************
;** 函数名称:   D_EZ_exti_config
;** 功能描述:   编码器EZ信号，外部输入
;** 输入参数: 
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-12
;** 修改原因：
;** 说    明:  	马达零点定位，使用外部中断处理 PB3
;***************************************************************************/
void D_EZ_exti_config(void) 
{
    gpio_init_type gpio_init_struct;
    // 配置外部中断
    exint_init_type exti_init_struct;
    // 启用GPIOA时钟
	crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
    
    /* set default parameter */
	gpio_default_para_init(&gpio_init_struct);

	/* configure button pin as input with pull-up/pull-down */
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	gpio_init_struct.gpio_pins = GPIO_PINS_3;
	gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
	gpio_init(GPIOB, &gpio_init_struct);
    // 选择PB3作为EXTI3
    gpio_exint_line_config(GPIO_PORT_SOURCE_GPIOB, GPIO_PINS_SOURCE3);
    
    exint_default_para_init(&exti_init_struct);
    exti_init_struct.line_enable = TRUE;
    exti_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
    exti_init_struct.line_select = EXINT_LINE_3;
    exti_init_struct.line_polarity = EXINT_TRIGGER_RISING_EDGE;
    exint_init(&exti_init_struct);
    
    // 配置NVIC
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(EXINT3_IRQn, 2, 3);
}


