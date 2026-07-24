#include "project.h"

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

