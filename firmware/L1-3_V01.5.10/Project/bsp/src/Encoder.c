#include "project.h"
#include "Encoder.h"

#define ENCODER_TIMER      TMR4
#define ENCODER_GPIO_PORT  GPIOB
#define ENCODER_PIN_A      GPIO_PINS_6
#define ENCODER_PIN_B      GPIO_PINS_7

/***************************************************************************
;** 函数名称:  Encoder_AB_Init	
;** 功能描述:  初始化AB相编码器接口，配置GPIO和定时器
;** 输    入: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: 
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因    编码器接口增加
;** 说    明:    使用TMR4定时器，GPIOB_Pin6/Pin7作为编码器输入
;**             TMR4配置为编码器模式(2倍频)，计数器值即为编码器位置
;**             编码器模式A+双边沿触发，实现2倍频计数(一圈2000计数)
;***************************************************************************/
void Encoder_AB_Init(void)
{
    gpio_init_type gpio_init_struct;

    /* 开启 GPIOB 和 TMR4 时钟 */
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_TMR4_PERIPH_CLOCK, TRUE);

    /* PB7/PB6 输入，上拉 */
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = ENCODER_PIN_A | ENCODER_PIN_B;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(ENCODER_GPIO_PORT, &gpio_init_struct);
    
    tmr_base_init(ENCODER_TIMER, 0xFFFF, 0);
    tmr_cnt_dir_set(ENCODER_TIMER, TMR_COUNT_UP);
    tmr_encoder_mode_config(ENCODER_TIMER, TMR_ENCODER_MODE_C,
                            TMR_INPUT_BOTH_EDGE, TMR_INPUT_BOTH_EDGE);

    
    tmr_counter_value_set(ENCODER_TIMER, 0);
    tmr_counter_enable(ENCODER_TIMER, TRUE);
}

/***************************************************************************
;** 函数名称:  Encoder_AB_GetCount	
;** 功能描述:  获取AB编码器的当前计数值
;** 输    入: 	无
;** 输    出: 	无
;** 返 回 值: 	当前编码器计数器的值
;** 全局变量: 
;** 调用模块: 
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    
;***************************************************************************/
int32_t Encoder_AB_GetCount(void)
{
    return (int32_t)tmr_counter_value_get(ENCODER_TIMER);
}

/***************************************************************************
;** 函数名称:  Encoder_AB_ResetCount	
;** 功能描述:  复位AB编码器计数器值为0
;** 输    入: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: 
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    
;***************************************************************************/
void Encoder_AB_ResetCount(void)
{
    tmr_counter_value_set(ENCODER_TIMER, 0);
}
