
/****************************************************************************************
*
*	模块名称 :   TMC2209
*	文件名称 : 
*	版    本 :   V0.1
*	说    明 :   TMC2209驱动程序实现
*	修改记录 :
*   版 本 号 :  
*   日    期 :   2026-6-16  
*   作    者 :   G-D-L
*	说    明 :   初始版本，仅实现了基本的电机控制功能。
*
*****************************************************************************************/
#include "project.h"
PositionConfig position_config;

static int32_t current_position = 0;      // 当前位置（步数）
bool motor_busy = FALSE;                  // 电机忙标志


static volatile uint32_t step_counter = 0;  // 当前步数计数器
static volatile uint32_t target_steps = 0;  // 目标步数
static volatile uint8_t is_running = 0;     // 电机是否在运行

/* 编码器反馈修正相关变量 */
static int32_t encoder_start_count = 0;     // 运动开始时的编码器计数

/* 速度曲线控制变量 */
static volatile uint32_t current_step_freq = 0;    // 当前步进频率
static volatile uint32_t accel_start_freq = 8000;  // 启动频率 8kHz
static volatile uint32_t accel_max_freq = 60000;   // 最大频率 60kHz
static volatile uint32_t accel_steps = 60;         // 加速段步数
static volatile uint32_t decel_steps = 60;         // 减速段步数
static volatile uint8_t ramp_phase = 0;            // 0:加速 1:匀速 2:减速
static volatile uint8_t send_cnt = 0;             // 发送次数计数器
static volatile uint8_t compensation_flag = 0;    // 编码器反馈修正标志

motor_status_t motor_status = {0};

tmc2209_config_t tmc2209_config = {
    .slave_address = 0x03,
    .microstep = MICROSTEP_1_128, // 1/128步进  128x200=25600步/圈
    .run_current = 400,           // 260mA
    .hold_current = 80, // 80%保持电流
    .direction = DIR_CW,
    .run_mode = TMC_MODE_SPREADCYCLE,
    .enable = 0,
    .step_frequency = 1000,    // 1kHz默认频率
};

/* 全局引脚状态 */
static tmc_pin_states_t pin_states = {0}; // 引脚状态结构体

/***************************************************************************
;** 函数名称:  tmc2209_gpio_init
;** 功能描述:  初始化TMC2209相关GPIO引脚
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: crm_periph_clock_enable, gpio_init, gpio_bits_set, gpio_bits_reset
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    配置ENN、STEP、DIR、SPREAD等控制引脚
;***************************************************************************/
static void tmc2209_gpio_init(void)
{
    gpio_init_type gpio_init_struct;
    
    /* 使能GPIO时钟 */
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
    
    /* ENN引脚配置 - 输出 */
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_pins = TMC_ENN_PIN;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(TMC_ENN_PORT, &gpio_init_struct);
    
    /* STEP引脚配置 - 输出 */
    gpio_init_struct.gpio_pins = TMC_STEP_PIN;
    gpio_init(TMC_STEP_PORT, &gpio_init_struct);
    
        /* SPREAD引脚配置 - 输出 */
    gpio_init_struct.gpio_pins = M_SPREAD_PIN;
    gpio_init(M_SPREAD_PORT, &gpio_init_struct);
    
    /* DIR引脚配置 - 输出 */
    gpio_init_struct.gpio_pins = TMC_DIR_PIN;
    gpio_init(TMC_DIR_PORT, &gpio_init_struct);
    
    /* 等待发送缓冲区空 */
    gpio_bits_set(TMC_ENN_PORT, TMC_ENN_PIN);  // 默认禁用
    gpio_bits_reset(TMC_STEP_PORT, TMC_STEP_PIN);
    gpio_bits_reset(TMC_DIR_PORT, TMC_DIR_PIN);
    gpio_bits_reset(M_SPREAD_PORT, M_SPREAD_PIN);
}
/***************************************************************************
;** 函数名称:  tmc2209_uart_init
;** 功能描述:  初始化UART3用于TMC2209通信
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: crm_periph_clock_enable, gpio_init, nvic_priority_group_config, nvic_irq_enable, usart_init, usart_interrupt_enable, usart_enable
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    配置USART3用于TMC2209串口通信，波特率TMC_UART_BAUDRATE
;***************************************************************************/
static void tmc2209_uart_init(void)
{
    gpio_init_type gpio_init_struct;
    
    /* 使能UART时钟 */
    crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);  
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

    /* configure the usart1 tx pin */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_10;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(GPIOB, &gpio_init_struct);

    /* configure the usart1 rx pin */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = GPIO_PINS_11;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(GPIOB, &gpio_init_struct);

    /* config usart nvic interrupt */
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(USART3_IRQn, 1, 1);

    /* configure usart3 param */
    usart_init(USART3, TMC_UART_BAUDRATE, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART3, TRUE); /* enable usart tx */
    usart_receiver_enable(USART3, TRUE);    /* enable usart rx */

    /* enable usart1 interrupt */
    usart_interrupt_enable(USART3, USART_RDBF_INT, TRUE); /* enable rx interrupt */
//  usart_interrupt_enable(USART3, USART_TDBE_INT, TRUE); /* enable tx interrupt */
    usart_enable(USART3, TRUE);

}
/***************************************************************************
;** 函数名称:  tmc2209_timer_init
;** 功能描述:  初始化步进电机加减速控制
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: tmr_interrupt_enable, nvic_irq_enable, tmr_counter_enable
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    配置TMR3定时器用于生成步进脉冲
;***************************************************************************/
void tmc2209_timer_init(void)
{
    tmr_interrupt_enable(TMR3, TMR_OVF_INT, TRUE);  /* overflow interrupt enable */

    /* tmr3 interrupt nvic init */
    nvic_irq_enable(TMR3_GLOBAL_IRQn, 0, 2);

    /* enable tmr3 */
    tmr_counter_enable(TMR3, TRUE);
}
/***************************************************************************
;** 函数名称:  tmc2209_init
;** 功能描述:  初始化TMC2209电机驱动器
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: tmc2209_gpio_init, tmc2209_uart_init, load_config_from_flash, init_default_config, save_config_to_flash, tmc2209_timer_init
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    初始化GPIO、UART、定时器，并加载或保存配置参数
;***************************************************************************/
void tmc2209_init(void)
{
	DriverError err;
    /* 初始化GPIO */
    tmc2209_gpio_init();
    /* 初始化UART */
    tmc2209_uart_init();

    err = load_config_from_flash(); // 从Flash中加载配置参数
    if (err != DRV_OK) 
    {
        sprintfx("加载默认参数\r\n");

        init_default_config(); // 初始化默认配置参数

        save_config_to_flash(); // 保存默认参数到Flash
    }
    tmc2209_timer_init();  // PWM for MOTOR  M_STEP

}

/***************************************************************************
;** 函数名称:  tmc2209_set_direction
;** 功能描述:  初始化步进电机加减速控制
;** 输    入: 	dir: 方向 (DIR_CW-顺时针, DIR_CCW-逆时针)
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: gpio_bits_reset, gpio_bits_set
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    通过DIR引脚控制电机旋转方向
;***************************************************************************/
void tmc2209_set_direction(motor_direction_t dir)
{
    tmc2209_config.direction = dir;
    motor_status.direction = dir;
    motor_status.enabled= dir;
    if (dir == DIR_CW) {
        gpio_bits_reset(TMC_DIR_PORT, TMC_DIR_PIN);
    } else {
        gpio_bits_set(TMC_DIR_PORT, TMC_DIR_PIN);
    }
}

/***************************************************************************
;** 函数名称:  tmc2209_set_current
;** 功能描述:  设置电机电流
;** 输    入: 	current_ma: 运行电流(mA)
;**           hold_percent: 保持电流百分比(0-100)
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: tmc2209_uart_send_command
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    计算IRUN和IHOLD值并写入IHOLD_IRUN寄存器
;***************************************************************************/
void tmc2209_set_current(uint16_t current_ma, uint8_t hold_percent)
{
    uint32_t ihold_irun = 0;
    uint8_t irun, ihold;
    uint8_t uart_data[8];
    
    tmc2209_config.run_current = current_ma;
    tmc2209_config.hold_current = hold_percent;
    /* 计算IRUN值 (范围0-31) */
    irun = (current_ma * 31) / 800;  /* 假设最大电流800mA */
    if(irun > 31) irun = 31;
    if(irun < 1) irun = 1;
    
    /* 计算IHOLD值 */
    ihold = (irun * hold_percent) / 100;
    if(ihold < 1) ihold = 1;
    if(ihold > 31) ihold = 31;
    
    /* 构造IHOLD_IRUN寄存器值 */
    ihold_irun = (ihold & 0x1F) |              /* IHOLD[4:0] */
                 ((irun & 0x1F) << 8) |        /* IRUN[4:0] */
                 ((uint32_t)0x05 << 16);       /* IHOLDDELAY[3:0] = 5 */

//    ihold_irun = 0x000C090F;

    uart_data[0] = 0x05;  // 从机地址
    uart_data[1] = TMC2209_UART_ADDR;
    uart_data[2] = 0x10|0x80;  // 寄存器地址(IOIN)
    uart_data[3] = (ihold_irun >> 24) & 0xFF;
    uart_data[4] = (ihold_irun >> 16) & 0xFF;
    uart_data[5] = (ihold_irun >> 8) & 0xFF;
    uart_data[6] = ihold_irun & 0xFF;
    uart_data[7] = calculate_crc(uart_data, 7);
    tmc2209_uart_send_command(uart_data, 8);
}

/***************************************************************************
** 函数名称:  tmc2209_step_pulse
;** 功能描述:  步进电机驱动
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: gpio_bits_set, gpio_bits_reset
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    在STEP引脚产生一个上升沿触发步进，最小100ns，2=170ns  5=300ns 10 = 530ns 20 = 990ns
;***************************************************************************/
void tmc2209_step_pulse(void)
{
    volatile uint32_t i;
    gpio_bits_set(TMC_STEP_PORT, TMC_STEP_PIN);
    
    /* 等待发送缓冲区空 */
    for(i = 0; i < 10; i++);
    
    gpio_bits_reset(TMC_STEP_PORT, TMC_STEP_PIN);
}

/***************************************************************************
** 函数名称:  tmc2209_set_motor_direction
** 全局变量: 
** 输    入: 	dir: 方向 (0-正向/DIR_CW, 1-反向/DIR_CCW)
** 输    出: 	无
** 返 回 值: 	驱动器状态结构体
** 全局变量: 
** 调用模块: gpio_bits_set, gpio_bits_reset
** 作    者:   G-D-L
** 日    期:   2026-6-12
** 修改原因
** 说    明:    通过DIR引脚设置电机旋转方向
;***************************************************************************/
void tmc2209_set_motor_direction(uint8_t dir)
{
    if(dir) {
        gpio_bits_set(TMC_DIR_PORT, TMC_DIR_PIN);
        motor_status.direction = DIR_CCW;
    } else {
        gpio_bits_reset(TMC_DIR_PORT, TMC_DIR_PIN);
        motor_status.direction = DIR_CW;
    }
}
/***************************************************************************
;** 函数名称: 	tmc2209_enable_motor
;** 功能描述:   使能电机
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void tmc2209_enable_motor(void)
{
    gpio_bits_reset(TMC_ENN_PORT, TMC_ENN_PIN);
    motor_status.enabled = 1;
    tmc2209_config.enable = 1;
}
/***************************************************************************
;** 函数名称: 	tmc2209_disable_motor
;** 功能描述:   禁用电机
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void tmc2209_disable_motor(void)
{
    gpio_bits_set(TMC_ENN_PORT, TMC_ENN_PIN);
    motor_status.enabled = 0;
    motor_status.moving = 0;
	motor_status.homing = 0;
    tmc2209_config.enable = 0;
}
/***************************************************************************
;** 函数名称: 	tmc2209_generate_step
;** 功能描述:   生成步进脉冲
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void tmc2209_generate_step(uint16_t pulse_width_us)
{
	volatile uint16_t i;
    gpio_bits_set(TMC_STEP_PORT, TMC_STEP_PIN);
    
    /* 等待发送缓冲区空 */
    for(i = 0; i < pulse_width_us; i++) {
        __NOP();
    }
    
    gpio_bits_reset(TMC_STEP_PORT, TMC_STEP_PIN);
    motor_status.moving = 1;
}
/***************************************************************************
;** 函数名称: 	tmc2209_stop_motion
;** 功能描述:   停止运动
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void tmc2209_stop_motion(void)
{
    tmr_counter_enable(STEP_TIMER, FALSE);
    is_running = 0;
    motor_status.moving=0;
    step_counter = 0;
    target_steps = 0;
    motor_busy = false;
    BUSY(0);
    //sprintfx("ok\r\n");
   // motor_status.position=motor_status.target_position;
}
/***************************************************************************
;** 函数名称: 	TMR3_GLOBAL_IRQHandler
;** 功能描述:   定时器中断服务函数
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void TMR3_GLOBAL_IRQHandler(void)
{
    if(tmr_flag_get(STEP_TIMER, TMR_OVF_FLAG) != RESET) {
        /* 发送步进脉冲 */
        tmc2209_step_pulse();
        step_counter++;

        if(compensation_flag == 0)
        {
            /* 动态频率调整 */
            if(step_counter <= accel_steps) {
                // 加速段：线性增加频率
                current_step_freq = accel_start_freq +
                    (accel_max_freq - accel_start_freq) * step_counter / accel_steps;
                ramp_phase = 0;
            }
            else if(step_counter <= (target_steps - decel_steps)) {
                // 匀速段：保持最大频率
                current_step_freq = accel_max_freq;
                ramp_phase = 1;
            }
            else if(step_counter < target_steps) {
                // 减速段：线性降低频率到接近0速
                uint32_t decel_progress = step_counter - (target_steps - decel_steps);
                current_step_freq = accel_max_freq -
                    (accel_max_freq - 500) * decel_progress / decel_steps;  // 降低到500Hz减少抖动
                ramp_phase = 2;
            }

            /* 更新定时器频率 */
            if(step_counter < target_steps) {
                tmc2209_timer_set_frequency(current_step_freq);
            }
//			tmc2209_config.step_frequency = current_step_freq;
        }     

        /* 到达目标步数停止 */
        if(target_steps > 0 && step_counter >= target_steps) {
            tmc2209_stop_motion();
            compensation_flag = 0;
        }

        tmr_flag_clear(STEP_TIMER, TMR_OVF_FLAG);
    }
}
/***************************************************************************
;** 函数名称: 	tmc2209_timer_set_frequency
;** 功能描述:   设置定时器频率
;** 输入参数:  frequency_hz: 步进频率(Hz)
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void tmc2209_timer_set_frequency(uint32_t frequency_hz)
{
    uint32_t period;
    
    if(frequency_hz == 0) {
        tmr_counter_enable(STEP_TIMER, FALSE);
        return;
    }
    
    /* 等待发送缓冲区空 */
    /* 定时器时钟 = 72MHz / division = 1MHz */
    period = 1000000UL / frequency_hz;  // 1MHz / frequency
//	period = frequency_hz;  // 1MHz / frequency
    
    if(period < 2) period = 2;
    if(period > 65535) period = 65535;
    
    tmr_period_value_set(STEP_TIMER, period);
   // tmr_counter_enable(STEP_TIMER, TRUE);
}

/***************************************************************************
;** 函数名称: 	tmc2209_calculate_deviation
;** 功能描述:   计算电机位置偏差
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void tmc2209_calculate_deviation(void)
{
	delay_ms(500);
    /* 获取当前编码器位置 */
    int32_t current_encoder = Encoder_AB_GetCount();
    float expected_encoder = motor_status.position * (ENCODER_PER_REVOLUTION / STEPS_PER_REV);
    int32_t encoder_delta = current_encoder - expected_encoder;
    int32_t position_delta = motor_status.position - (current_encoder * STEPS_PER_REV / ENCODER_PER_REVOLUTION);
    
	sprintfx("%ld\r\n", position_delta);
//	sprintfx("通道号: %ld, 目标位置: %ld, 偏差微步数: %ld\r\n", motor_status.channelNo, motor_status.position, position_delta);
}

/***************************************************************************
;** 函数名称: 	tmc2209_wait_move_done
;** 功能描述:   等待运动完成
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
DriverError tmc2209_wait_move_done(uint16_t timeout_ms)
{
    uint32_t timeout = 0;
    DriverError ret = DRV_OK;

    while (is_running)
	{
		// 忙等待运动结束
//		timeout++;
//		if (timeout > timeout_ms)  // 超时保护10s
//		{
//            tmc2209_stop_motion();
//            sprintfx("电机运行超时\r\n");
//			tmc2209_stall_detected();
//			return DRV_ERR_HOMING_TIMEOUT;
//		}
//        delay_ms(1);
	}
    return ret;
}
/***************************************************************************
;** 函数名称: 	tmc2209_motion_compensation
;** 功能描述:   运动补偿
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void tmc2209_motion_compensation(void)
{
	/* 等待主运动完成 */
//	while(is_running){};
//		printfx(" pass\r\n");
//	delay_ms(100);
////	tmc2209_wait_move_done(10000);
//	
	/* 停止后进行编码器位置补偿 */
	int32_t current_encoder = Encoder_AB_GetCount();
	int32_t encoder_delta = current_encoder - encoder_start_count;
	
	/* 计算编码器对应的理论步数 */
	int32_t expected_steps = (encoder_delta * STEPS_PER_REV) / ENCODER_PER_REVOLUTION;
	int32_t error = motor_status.position - expected_steps;
	
	/* 误差超过阈值时进行补偿运动 */
	if (error > ENCODER_CORRECT_THRESHOLD || error < -ENCODER_CORRECT_THRESHOLD)
	{
		/* 计算需要补偿的步数和方向
		 * error > 0: 马达走多了，需要往回走
		 * error < 0: 马达走少了，需要继续往前走
		 */
		motor_direction_t correct_dir;
		uint32_t correct_steps;
		
		if (error < 0)
		{
			/* 走多了，反向补偿 */
			correct_dir = DIR_CCW;//(dir == DIR_CW) ? DIR_CCW : DIR_CW;
			correct_steps = (uint32_t)(-error);
		}
		else
		{
			/* 走少了，同向补偿 */
			correct_dir = DIR_CW;
			correct_steps = (uint32_t)(error);
		}
		
		/* 执行补偿运动（慢速） */
		if (correct_steps > 0)
		{
//				encoder_start_count = Encoder_AB_GetCount();  // 记录补偿前位置
			step_counter = 0;
			target_steps = correct_steps;
			
			tmc2209_set_direction(correct_dir);
//			compensation_flag = 1; // 设置补偿标志
			if (tmc2209_config.step_frequency != MOVE_COMP_SPEED)
			{
				tmc2209_config.step_frequency = MOVE_COMP_SPEED;
				tmc2209_timer_set_frequency(MOVE_COMP_SPEED);
				motor_status.speed = MOVE_COMP_SPEED;
			}
			tmr_counter_enable(STEP_TIMER, TRUE);
			is_running = 1;
			motor_status.moving = 1;
			BUSY(1);
			
			/* 等待补偿完成 */
			while(is_running){};
//			tmc2209_wait_move_done(10000);
		}
	}
}
/***************************************************************************
;** 函数名称: 	tmc2209_move_steps
;** 功能描述:   移动电机指定步数
;** 输入参数:    steps: 步进数量
;**                  frequency_hz: 步进频率
;**                  dir: 方向，DIR_CW/DIR_CCW
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void tmc2209_move_steps(uint32_t steps, uint32_t frequency_hz, motor_direction_t dir)
{
//    tmc2209_enable_motor();
    tmc2209_set_direction(dir);
    
    /* 记录编码器初始位置 */
    encoder_start_count = 0;//Encoder_AB_GetCount();
    if(dir == DIR_CW)
    {
        motor_status.position += steps; // 绝对位置
    }
    else
    {
        motor_status.position -= steps; // 绝对位置
    }

	target_steps = steps;

			step_counter = 0;
			if (tmc2209_config.step_frequency != frequency_hz)
			{
				tmc2209_config.step_frequency = frequency_hz;
				tmc2209_timer_set_frequency(frequency_hz);
				motor_status.speed = frequency_hz;
			}
			tmr_counter_enable(STEP_TIMER, TRUE);
			
			is_running = 1;
			motor_status.moving = 1;
			BUSY(1);
			while(is_running){};
//			tmc2209_uart_write_reg(0x22,-100000);
//			delay_ms(10);
//			tmc2209_uart_write_reg(0x22,0);
    
//	 if(motor_status.homing)
//	 {
//		for(uint8_t i = 0;i < ENCODER_FEEDBACK_COMPENSATION_TIMES;i++)
//		{
//			tmc2209_motion_compensation(); // 运动补偿          		
//		}
//	 }
}

/***************************************************************************;** 函数名称:  tmc2209_move_steps_ramp
;** 功能描述:  带加减速曲线的步进电机控制
;** 输入参数:  steps: 步进数量, max_freq: 最大频率, dir: 方向
;** 返 回 值:  无
;** 说    明:  适用于短距离快速移动，防止过冲
;***************************************************************************/
void tmc2209_move_steps_ramp(uint32_t steps, uint32_t max_freq, motor_direction_t dir)
{
	/* 参数合理性检查 */
	if(steps == 0) return;

	/* 根据步数动态调整加减速段长度 */
	if(steps < 200) {
		accel_steps = steps / 6;      // 短距离：加速段占16.7%
		decel_steps = steps / 4;      // 减速段占25%，更长减速减少抖动
	} else {
		accel_steps = steps / 8;      // 长距离：加速段占12.5%
		decel_steps = steps / 4;      // 减速段占20%，增加减速段长度
	}

	/* 确保至少有一定加减速步数 */
	if(accel_steps < 20) accel_steps = 20;
	if(decel_steps < 20) decel_steps = 20;
	if(accel_steps + decel_steps > steps / 2) {
		accel_steps = steps / 4;
		decel_steps = steps / 4;
	}

	/* 设置速度参数 */
	accel_max_freq = max_freq;
	accel_start_freq = max_freq / 6;  // 启动频率为最大频率的1/6
	if(accel_start_freq < 5000) accel_start_freq = 5000;

	tmc2209_set_direction(dir);

	/* 更新位置记录 */
	if(dir == DIR_CW) {
		motor_status.position += steps;
	} else {
		motor_status.position -= steps;
	}

	/* 初始化运动参数 */
	step_counter = 0;
	target_steps = steps;
	current_step_freq = accel_start_freq;
	ramp_phase = 0;

	/* 设置初始频率并启动 */
	tmc2209_timer_set_frequency(accel_start_freq);
	motor_status.speed = accel_start_freq;

	tmr_counter_enable(STEP_TIMER, TRUE);
	is_running = 1;
	motor_status.moving = 1;
	BUSY(1);
    send_cnt++;

	/* 等待运动完成 */
	while(is_running) {};

    // /* 拉低驱动器使能引脚2ms再恢复锁定 */
	// gpio_bits_set(TMC_ENN_PORT, TMC_ENN_PIN);
	// delay_ms(1);
	// gpio_bits_reset(TMC_ENN_PORT, TMC_ENN_PIN);

	/* 运动完成后延时稳定电机，减少抖动 */
	delay_ms(50);  // 等待50ms让电机完全稳定

	/* 运动完成后补偿 */
//	if(((motor_status.homing) && (send_cnt % 2 == 1))) {
	if(motor_status.homing) {
		for(uint8_t i = 0; i < ENCODER_FEEDBACK_COMPENSATION_TIMES; i++) {
			tmc2209_motion_compensation();
		}
//		tmc2209_calculate_deviation();
	}
}
/***************************************************************************
;** 函数名称:  tmc2209_homing
;** 功能描述:  初始化步进电机加减速控制
;** 功能描述:  初始化步进电机加减速控制
;** 返 回 值:
;** 全局变量: 
;** 全局变量: 
;** 返 回 值: 	无
;** 日　  期:  2026-6-12
;** 修改原因
;** 说    明:  编码器EZ脚到达归零点产生脉冲中断
;***************************************************************************/
DriverError tmc2209_homing(uint32_t speed_hz)
{
    uint32_t timeout = 0;
    volatile uint32_t i;
	uint32_t delay_us;
    tmc2209_enable_motor();
    BUSY(1);
	pin_states.org_state = g_origin_location_flag;

	tmc2209_move_steps(HOMEING_OFFSET, HOMEDING_SPEED_1, DIR_CCW); // 先往反方向移动200微步，保证离开归零位
	while(is_running){};
//	tmc2209_wait_move_done(10000);

	g_origin_location_flag = 0;
    pin_states.org_state   = 0;
/* 全局引脚状态 */
    tmc2209_set_motor_direction(DIR_CW);  // DIR_CW为归零方向

    /* 等待发送缓冲区空 */
    timeout=0;
    while(!pin_states.org_state) //第一次归零  粗归零  编码器值0~25
    {
        tmc2209_step_pulse();
        
        /* 发送数据 */
        delay_us = 1000000 / speed_hz;
        for(i = 0; i < delay_us; i++) {
            __NOP();
        }
        timeout++;
        if(timeout > 40000) {  // 超时
           motor_status.homing = 0;
           motor_status.position = 0;
           motor_status.target_position= 0;
           tmc2209_disable_motor();

           return DRV_ERR_HOMING_TIMEOUT;
        }
		pin_states.org_state = g_origin_location_flag;
		
		if(timeout == 200) pin_states.org_state = 1;
    }

//	tmc2209_disable_motor();
    Encoder_AB_ResetCount();

    /* 设置当前位置为0 */
	g_origin_location_flag = 0;
    motor_status.position = 0;
    motor_status.target_position= 0;

    return DRV_OK;
}
/***************************************************************************
;** 函数名称:  tmc2209_stallguard_config
;** 功能描述:  初始化TMC2209电机驱动器
;** 功能描述:  初始化步进电机加减速控制
;** 返 回 值:
;** 全局变量: 
;** 全局变量: 
;** 作　  者:  G-D-L 
;** 日　  期:  2026-6-12
;** 修改原因
;** 说    明:  禁能后disable motor直接刹停会导致堵转
;***************************************************************************/
void tmc2209_stallguard_config(void) 
{
	uint32_t data;
    DriverError ret = DRV_OK;
	delay_ms(10);
//	if(tmc2209_config.run_mode == TMC_MODE_SPREADCYCLE)
//	{M_SPREAD_PORT->scr = M_SPREAD_PIN;}
//	else
//	{M_SPREAD_PORT->clr = M_SPREAD_PIN;}
    delay_ms(10);
    tmc2209_uart_write_reg(TMC2209_REG_GCONF,0x00000090);//92
    delay_ms(10);
    tmc2209_uart_write_reg(TMC2209_REG_GCONF,0x00000090);//92    
    delay_ms(10);
    tmc2209_set_current(800, 50);      // 720mA运行电流，80%保持电流
    delay_ms(10);
    tmc2209_uart_write_reg(TMC2209_REG_TPWMTHRS,0x00000000);
    delay_ms(10);
    tmc2209_uart_write_reg(TMC2209_REG_TCOOLTHRS,0x00000064);
    delay_ms(10);
    tmc2209_uart_write_reg(TMC2209_WREG_SGTHRS,0x00000096);
    delay_ms(10);
    tmc2209_uart_write_reg(TMC2209_REG_CHOPCONF,0x01000053);
    delay_ms(10);
    tmc2209_uart_write_reg(TMC2209_REG_PWMCONF,0xC10D1624);
    delay_ms(10);
//	tmc2209_uart_read_reg(TMC2209_REG_CHOPCONF,&data);
//	delay_ms(10);
	motor_status.homing = 0;
    ret = move_to_home();
    if(ret != DRV_OK)
    {
        sprintfx("Homeing failed\r\n");
        tmc2209_disable_motor();
        return;
    }
    else 
    {
        printfx("Homeing success\r\n");
    }
}
/***************************************************************************
;** 函数名称:  tmc2209_stall_detected
;** 功能描述:  
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 调用模块: 无
;** 调用模块: gpio_bits_write, delay_ms
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    检测到堵转时点亮LED提示
;***************************************************************************/
void tmc2209_stall_detected(void) 
{
    // gpio_bits_write(GPIOC, GPIO_PINS_13, FALSE); // 点亮LED
    S_LED(OFF);

    // delay_ms(100);

    // gpio_bits_write(GPIOC, GPIO_PINS_13, TRUE); // 关闭LED
}
/***************************************************************************
;** 函数名称:  tmc2209_stall_clear
;** 功能描述:  
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 调用模块: 无
;** 调用模块: 
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:   清除堵转灭LED灯
;***************************************************************************/
void tmc2209_stall_clear(void) 
{
    // gpio_bits_write(GPIOC, GPIO_PINS_13, FALSE); // 点亮LED
    S_LED(ON);

    // delay_ms(100);

    // gpio_bits_write(GPIOC, GPIO_PINS_13, TRUE); // 关闭LED
}
/***************************************************************************
;** 函数名称:  calculate_crc32
;** 功能描述:  CRC32计算函数
;** 输    入: 	data: 要发送的数据
;**           length: 数据长度
;** 输    出: 	无
;** 返 回 值: 	CRC32校验值
;** 调用模块: 无
;** 调用模块: 无
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    计算数据的CRC32校验值
;***************************************************************************/
static uint32_t calculate_crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
	size_t i;
	int j;
    
    for (i = 0; i < length; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return ~crc;
}
/***************************************************************************
;** 函数名称:  init_default_config
;** 功能描述:  初始化步进电机加减速控制
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: position_config
;** 调用模块: memset
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    初始化步进电机加减速参数
;***************************************************************************/
static void init_default_config(void) 
{
	int i;
    memset(&position_config, 0, sizeof(position_config));
    

    position_config.version = 1;
    

    position_config.points[POINT_HOME].step_count = 0;
    position_config.points[POINT_HOME].calibrated = true;
    position_config.points[POINT_HOME].calibration_time = 0;
    
    position_config.points[POINT_P1].step_count = 720;
    position_config.points[POINT_P1].calibrated = true;
    
    position_config.points[POINT_P8].step_count = 6358;
    position_config.points[POINT_P8].calibrated = true;
    
    position_config.points[POINT_P16].step_count = STEPS_PER_REV;
    position_config.points[POINT_P16].calibrated = true;
    

    for (i = 0; i < MAX_POINTS; i++) 
    {
        if (!position_config.points[i].calibrated) {
            position_config.points[i].step_count = 0;
            position_config.points[i].calibration_time = 0;
        }
    }
    position_config.max_steps_num = STEPS_PER_REV;
    position_config.last_save_time=1;
}
/***************************************************************************
;** 函数名称:  load_config_from_flash
;** 功能描述:  从Flash加载配置
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	DriverError错误码
;** 全局变量: position_config
;** 调用模块: W25Q128_ReadMulByte, memcpy, calculate_crc32
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    从Flash读取位置配置并验证CRC
;***************************************************************************/
static DriverError load_config_from_flash(void) 
{
    uint8_t flash_buffer[sizeof(PositionConfig)];
	uint32_t calculated_crc;
    
    W25Q128_ReadMulByte(FLASH_CONFIG_ADDR,flash_buffer, sizeof(PositionConfig));

    memcpy(&position_config, flash_buffer, sizeof(PositionConfig));
    

    if (position_config.version == 0xFFFFFFFF) {
        return DRV_ERR_CONFIG_CORRUPT;
    }
    

    calculated_crc = calculate_crc32((uint8_t*)&position_config, sizeof(position_config) - sizeof(position_config.crc32));
    
    if (calculated_crc != position_config.crc32) {
        return DRV_ERR_CONFIG_CORRUPT;
    }
    
    return DRV_OK;
}
/***************************************************************************
;** 函数名称:  save_config_to_flash
;** 功能描述:  保存配置到Flash
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	DriverError错误码
;** 全局变量: position_config
;** 调用模块: calculate_crc32, Save_position_config
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    计算CRC并保存位置配置到Flash
;***************************************************************************/
DriverError save_config_to_flash(void) {


    position_config.crc32 = calculate_crc32(
        (uint8_t*)&position_config, 
        sizeof(position_config) - sizeof(position_config.crc32)
    );
    Save_position_config(FLASH_CONFIG_ADDR,(uint8_t*)&position_config);
    return DRV_OK;
}
/***************************************************************************
;** 函数名称:  calculate_steps_for_point
;** 功能描述:  初始化步进电机加减速控制
;** 输    入: 	point: 位置点
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: position_config
;** 调用模块: 无
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 输    出: 	无
;***************************************************************************/
static int32_t calculate_steps_for_point(PositionPoint point) 
{
	int i;
	int32_t interpolated_steps;
	int32_t lower_steps,upper_steps;
	int lower_calibrated = -1; //下边界
    int upper_calibrated = -1;//上边界
	double step_per_point;

    if (point >= MAX_POINTS) {
        return -1;
    }
    

    if (position_config.points[point].calibrated) {
        return position_config.points[point].step_count;
    }
    


    for (i = point - 1; i >= 0; i--) {
        if (position_config.points[i].calibrated) {
            lower_calibrated = i;
            break;
        }
    }
    

    for (i = point + 1; i < MAX_POINTS; i++) {
        if (position_config.points[i].calibrated) {
            upper_calibrated = i;
            break;
        }
    }
    

    if (lower_calibrated == -1 && upper_calibrated == -1) {
        return -1;
    }
    

    if (upper_calibrated == -1) {
        return position_config.points[lower_calibrated].step_count;
    }
    

    if (lower_calibrated == -1) {
        return position_config.points[upper_calibrated].step_count;
    }
    

    lower_steps = position_config.points[lower_calibrated].step_count;
    upper_steps = position_config.points[upper_calibrated].step_count;
    
    step_per_point = (double)(upper_steps - lower_steps) / 
                           (upper_calibrated - lower_calibrated);
    
    interpolated_steps = lower_steps + 
                                (int32_t)(step_per_point * (point - lower_calibrated) + 0.5);
    
    return interpolated_steps;
}
/***************************************************************************
;** 函数名称:  is_point_calibrated
;** 功能描述:  初始化步进电机加减速控制
;** 输    入: 	point: 位置点
;** 输    出: 	无
;** 返 回 值: 	true-已校准，false-未校准
;** 全局变量: position_config
;** 调用模块: 无
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 输    出: 	无
;***************************************************************************/
bool is_point_calibrated(PositionPoint point) 
{
    if (point >= MAX_POINTS) {
        return false;
    }
    
    return position_config.points[point].calibrated;
}
/***************************************************************************
;** 函数名称:  save_position_config
;** 功能描述:  初始化步进电机加减速控制
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	DriverError错误码
;** 调用模块: 无
;** 调用模块: save_config_to_flash
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    保存位置配置到Flash
;***************************************************************************/
DriverError save_position_config(void) 
{
    return save_config_to_flash();
}
/***************************************************************************
;** 函数名称:  load_position_config
;** 功能描述:  初始化步进电机加减速控制
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	DriverError错误码
;** 调用模块: 无
;** 调用模块: load_config_from_flash
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    从Flash加载位置配置
;***************************************************************************/
DriverError load_position_config(void) 
{
    return load_config_from_flash();
}
/***************************************************************************
;** 函数名称:  calibrate_point
;** 功能描述:  初始化步进电机加减速控制
;** 输    入: 	point: 位置点
;**           steps: 步数
;** 输    出: 	无
;** 返 回 值: 	DriverError错误码
;** 全局变量: position_config
;** 调用模块: save_position_config
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:  记录位置到Flash
;***************************************************************************/
DriverError calibrate_point(PositionPoint point, int32_t steps) 
{
    if (point >= MAX_POINTS) 
    {
        return DRV_ERR_INVALID_POINT;
    }
    
    if (steps < 0 || steps > position_config.max_steps_num) 
    {
        return DRV_ERR_INVALID_POINT;
    }
    
    position_config.points[point].step_count = steps;
    position_config.points[point].calibrated = true;
    position_config.points[point].calibration_time = 1;//get_system_time();
    

    return save_position_config();
}
/***************************************************************************
;** 函数名称:  get_point_steps
;** 功能描述:  初始化步进电机加减速控制
;** 输    入: 	point: 位置点
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: position_config
;** 调用模块: calculate_steps_for_point
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 输    出: 	无
;***************************************************************************/
int32_t get_point_steps(PositionPoint point) 
{
    if (point >= MAX_POINTS) {
        return -1;
    }
    
    if (position_config.points[point].calibrated) {
        return position_config.points[point].step_count;
    }
    
    return calculate_steps_for_point(point);
}
/***************************************************************************
;** 函数名称:  clear_calibration
;** 功能描述:  初始化步进电机加减速控制
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	DriverError错误码
;** 全局变量: position_config
;** 调用模块: save_position_config
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    初始化步进电机加减速参数
;***************************************************************************/
DriverError clear_calibration(void) 
{
	int i;
    for (i = 0; i < MAX_POINTS; i++) {
        position_config.points[i].calibrated = false;
        position_config.points[i].step_count = 0;
        position_config.points[i].calibration_time = 0;
    }
    

    position_config.points[POINT_HOME].step_count = 0;
    position_config.points[POINT_HOME].calibrated = true;
    
    return save_position_config();
}
/***************************************************************************
;** 函数名称:  get_calibrated_point_count
;** 功能描述:  初始化步进电机加减速控制
;** 输    出: 	无
;** 输    出: 	无
;** 返 回 值: 	无
;** 全局变量: position_config
;** 调用模块: 无
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 输    出: 	无
;***************************************************************************/
int get_calibrated_point_count(void) 
{
    int count = 0;
	int i;
    for (i = 0; i < MAX_POINTS; i++) {
        if (position_config.points[i].calibrated) {
            count++;
        }
    }
    return count;
}
/***************************************************************************
;** 函数名称:  Save_position_config
;** 功能描述:  保存配置到Flash
;** 输    入: 	addr: Flash地址
;**           par: 数据指针
;** 输    出: 	无
;** 返 回 值: 	无
;** 调用模块: 无
;** 调用模块: W25Q128_SectorErase, W25Q128_WriteMulByte
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    初始化步进电机加减速参数
;***************************************************************************/
void Save_position_config(uint32_t addr,uint8_t *par)
{
    W25Q128_SectorErase(addr);
    W25Q128_WriteMulByte(addr,par,sizeof(PositionConfig));
}
/***************************************************************************
;** 函数名称:  move_to_point
;** 功能描述:  初始化步进电机加减速控制
;** 输    入: 	point: 位置点
;** 输    出: 	无
;** 返 回 值: 	DriverError错误码
;** 全局变量: motor_busy
;** 调用模块: get_point_steps, move_to_absolute_step
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 输    出: 	无
;***************************************************************************/
DriverError move_to_point(PositionPoint point) 
{
	int32_t target_steps;
    if (motor_busy) {
        return DRV_ERR_MOTOR_BUSY;
    }
    if (point >= MAX_POINTS) {
        return DRV_ERR_INVALID_POINT;
    }
    target_steps = get_point_steps(point);
    //sprintfx("target_steps:%d\r\n",target_steps);
    if (target_steps < 0) {
        return DRV_ERR_INVALID_POINT;
    }
    
    return move_to_absolute_step(point,target_steps);
}
/***************************************************************************
;** 函数名称:  move_to_absolute_step
;** 功能描述:  初始化步进电机加减速控制
;** 输    入: 	point: 位置点
;**           targetstep: 目标步数
;** 输    出: 	无
;** 返 回 值: 	DriverError错误码
;** 全局变量: motor_busy, current_position, position_config
;** 调用模块: set_direction, delay_us, tmr_counter_enable, tmr_period_value_set
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    初始化步进电机加减速参数
;***************************************************************************/
DriverError move_to_absolute_step(PositionPoint point,int32_t absolute_step) 
{
	int32_t abs_steps,steps_to_move;
	motor_direction_t forward;
   // int LastDir=DIR_CCW;
    if (motor_busy) 
    {
        return DRV_ERR_MOTOR_BUSY;
    }
    
    if (absolute_step < 0 || absolute_step > position_config.max_steps_num) 
    {
        return DRV_ERR_INVALID_POINT;
    }
    
    motor_busy = true;
    BUSY(1);
#if 1
    tmc2209_enable_motor();

    steps_to_move = absolute_step - motor_status.position;
    if (steps_to_move < 0) 
    {
        forward = DIR_CW;
    } 
    else 
    {
        forward = DIR_CCW;
    }
    
    if (steps_to_move == 0) 
    {
        motor_busy = false;
        BUSY(0);
        return DRV_OK;
    }
//    tmc2209_set_motor_direction(forward);

    abs_steps = (steps_to_move > 0) ? steps_to_move : -steps_to_move;

#if 0

    for (uint32_t i = 0; i < abs_steps; i++) 
		{
        tmc2209_step_pulse();
        

        if (forward) {
            current_position++;
        } else {
            current_position--;
        }
        


        delay_us(1000000 / 1000);
    }
    motor_status.position=current_position;
    is_running = 0;
    motor_status.moving=0;
    motor_busy = false;
    BUSY(0);
#else
/* 全局引脚状态 */
    // tmc2209_enable_motor();
//	motor_status.position = absolute_step;
    target_steps = abs_steps;
    step_counter = 0;
    tmc2209_set_motor_direction(forward);
    tmc2209_move_steps(target_steps, MOVE_SPEED, forward);
//	while(is_running){};
//    tmc2209_wait_move_done(10000);
	motor_status.channelNo=point;//保存当前通道号
//    tmc2209_calculate_deviation();
#endif
#else
    steps_to_move = targetstep - current_position;
    forward = (steps_to_move ? 1: 0);
	if(steps_to_move < 0)
	{forward = 0;}
	else {forward = 1;}
    
    if (steps_to_move == 0) 
    {
        motor_busy = false;
        BUSY(0);
        return DRV_OK;
    }
    delay_us(100);
    abs_steps = (steps_to_move > 0) ? steps_to_move : -steps_to_move;
    target_steps = abs_steps;
    step_counter = 0;
	tmc2209_set_motor_direction(forward);
    delay_us(100);
    tmc2209_move_steps(target_steps, 1000, forward);
    tmc2209_wait_move_done(10000);
    current_position = targetstep;
    motor_status.position = current_position;
    is_running = 0;
    motor_status.moving = 0;
    motor_busy = false;
    BUSY(0);
#endif
    motor_status.channelNo=point;//保存当前通道号
    return DRV_OK;
}
/***************************************************************************
;** 函数名称:  move_to_home
;** 功能描述:  MCU上电初始化 使马达移动到归零点
;** 功能描述:  初始化步进电机加减速控制
;** 返 回 值:  DRV_OK
;** 全局变量: 
;** 全局变量: 
;** 作　  者:  G-D-L 
;** 日　  期:  2026-6-12
;** 修改原因
;** 说    明:
;***************************************************************************/
DriverError move_to_home(void) 
{
    DriverError ret = DRV_OK;
    ret = tmc2209_homing(HOMEDING_SPEED); // 调整归零时电机速度，数值越大越快，越慢越准确
    if(ret == DRV_ERR_HOMING_TIMEOUT)
    {
        motor_busy = true;
        BUSY(1);
        return ret;
    }
	ret = tmc2209_homing(10);
    motor_status.channelNo=0;
    current_position = 0;
	motor_status.homing = 1;
    motor_busy = false;
    BUSY(0);
	tmc2209_stall_clear();
    return DRV_OK;
}
