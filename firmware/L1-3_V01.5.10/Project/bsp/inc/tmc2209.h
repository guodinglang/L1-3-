/**
 * @file tmc2209.h
 * @brief TMC2209驱动程序头文件
 * @version 1.0
 */

#ifndef __TMC2209_H
#define __TMC2209_H

#include "at32f403a_407.h"
#include "at32f403a_407_gpio.h"
#include <stdbool.h>

#define MS_TICK                    (system_core_clock / 1000U)
#define DELAY                      100 /* 200 ms ticks */

/* 硬件连接配置 - 根据实际硬件修改 */
#define TMC_UART_PORT              GPIOB
#define TMC_UART_PIN               GPIO_PINS_10
#define TMC_UART_CTS_PIN           GPIO_PINS_11
#define TMC_UART                   USART3
#define TMC_UART_IRQn              USART3_IRQn
#define TMC_UART_IRQHANDLER        USART3_IRQHandler
#define TMC_UART_BAUDRATE          115200

/* ENN引脚配置 */
#define TMC_ENN_PORT               GPIOB
#define TMC_ENN_PIN                GPIO_PINS_2

/* STEP/DIR引脚配置 */
#define TMC_STEP_PORT              GPIOB
#define TMC_STEP_PIN               GPIO_PINS_0
#define TMC_DIR_PORT               GPIOB
#define TMC_DIR_PIN                GPIO_PINS_1

/* 归零位置引脚 - PA12 */
#define M_ORG_PORT                  GPIOA
#define M_ORG_PIN                   GPIO_PINS_12
#define M_ORG_PIN_SOURCE            GPIO_PINS_SOURCE12

/* 诊断和堵转状态输出引脚 - PA11 */
#define M_DIAG_PORT                 GPIOA
#define M_DIAG_PIN                  GPIO_PINS_11
#define M_DIAG_PIN_SOURCE           GPIO_PINS_SOURCE11

/* 输出位置信号引脚 - PA8 */
#define M_INDEX_PORT                GPIOA
#define M_INDEX_PIN                 GPIO_PINS_8
#define M_INDEX_PIN_SOURCE          GPIO_PINS_SOURCE8

/* 运行模式控制引脚 - PA7 */
#define M_SPREAD_PORT               GPIOA
#define M_SPREAD_PIN                GPIO_PINS_7
#define M_SPREAD_PIN_SOURCE         GPIO_PINS_SOURCE7
///* MS1, MS2引脚配置 */
//#define TMC_MS1_PORT               GPIOB
//#define TMC_MS1_PIN                GPIO_PINS_3
//#define TMC_MS2_PORT               GPIOB
//#define TMC_MS2_PIN                GPIO_PINS_4
// TMC2209寄存器地址
#define TMC2209_SLAVE_ADDR     0x03
#define TMC2209_READ_BIT       0x00
#define TMC2209_WRITE_BIT      0x80

// 寄存器地址
#define REG_GCONF          0x00
#define REG_IHOLD_IRUN     0x10
#define REG_CHOPCONF       0x6C
#define REG_PWMCONF        0x70


/* 定时器配置 */
#define STEP_TIMER               TMR3
#define STEP_TIMER_PERIPH_CLOCK  CRM_TMR3_PERIPH_CLOCK
#define STEP_TIMER_IRQn          TMR3_GLOBAL_IRQn
#define STEP_TIMER_IRQHANDLER    tmr3_global_isr

// 电机参数配置
#define MICROSTEPS        128    // 微步细分
#define STEPS_PER_REV     25600  // 每转步数
//#define MAX_STEP_POSITION 12736  // P16位置

// 移动速度（Hz）
#define HOMEDING_SPEED_1   500    // 归零移动速度（Hz）
#define HOMEDING_SPEED   500    // 归零移动速度（Hz）
#define MOVE_SPEED       50000   // 移动速度（Hz）
#define MOVE_COMP_SPEED  5000   // 补偿移动速度（Hz）
#define HOMEING_OFFSET   300    // 归零偏移步数（微步）

#define ENCODER_PER_REVOLUTION  4000        // 编码器一圈脉冲数
#define ENCODER_CORRECT_INTERVAL 100         // 每50步检查一次编码器
#define ENCODER_CORRECT_THRESHOLD 3         // 修正阈值（步数） 25600/4000=6.4步/脉冲，3步误差约等于半个编码器脉冲
#define ENCODER_FEEDBACK_COMPENSATION_TIMES 1 // 反馈补偿次数

#define COMPENSATION_TIME  50


/* 工作模式 */
typedef enum {
    MODE_UART = 0,      // UART模式
    MODE_STEP_DIR,      // STEP/DIR模式
} tmc2209_mode_t;

/* 微步模式 */
typedef enum {
    MICROSTEP_FULL = 0,     // 全步
    MICROSTEP_HALF,         // 1/2步
    MICROSTEP_1_4,          // 1/4步
    MICROSTEP_1_8,          // 1/8步
    MICROSTEP_1_16,         // 1/16步
    MICROSTEP_1_32,         // 1/32步
    MICROSTEP_1_64,         // 1/64步
    MICROSTEP_1_128,        // 1/128步
    MICROSTEP_1_256,        // 1/256步
} microstep_mode_t;
/**
 * @brief 步进电机参数结构体
 */
typedef struct {
    float steps_per_rev;      // 每转步数
    float step_angle;         // 每一步的角度（度）
    uint32_t microsteps;      // 细分数
    int32_t current_steps;    // 当前步数（可正可负）
} StepperMotor_t;
/* 电机方向 */
typedef enum {
    DIR_CW = 0,     // 顺时针
    DIR_CCW = 1,    // 逆时针
} motor_direction_t;

/* 运行模式 */
typedef enum {
    STEALTHCHOP = 0,
    SPREADCYCLE = 1,
} run_mode_t;

/* 驱动状态结构体 */
typedef struct {
    uint8_t driver_error;       // 驱动错误标志
    uint8_t stall_flag;         // 失速标志
    uint8_t over_temp_warning;  // 过温警告
    uint8_t over_temp_shutdown; // 过温保护
    uint8_t open_load_a;        // A相开路
    uint8_t open_load_b;        // B相开路
    uint8_t short_to_gnd_a;     // A相对地短路
    uint8_t short_to_gnd_b;     // B相对地短路
} tmc2209_status_t;
/* 运行模式 - 根据图片表格：M_SPERAD引脚控制 */
typedef enum {
    TMC_MODE_STEALTHCHOP = 0,  /* Low=StealthChop */
    TMC_MODE_SPREADCYCLE = 1   /* High=SpreadCycle */
} tmc_run_mode_t;

///* 驱动配置结构体 */
//typedef struct {
//    microstep_mode_t microstep;  // 微步模式
//    uint16_t current;            // 电流值(mA)
//    uint8_t hold_current_percent; // 保持电流百分比
//    motor_direction_t direction;  // 方向
//    run_mode_t run_mode;         // 运行模式
//    uint8_t enable;              // 使能状态
//    uint32_t step_frequency;     // 步进频率(Hz)
//} tmc2209_config_t;
/* 驱动配置结构体 */
typedef struct {
    uint8_t slave_address;              /* 从机地址 */
    microstep_mode_t microstep;         /* 微步设置(1,2,4,8,16,32,64,128,256) */
    uint16_t run_current;               /* 运行电流(mA) */
    uint8_t hold_current;               /* 保持电流百分比(%) */
    motor_direction_t direction;        /*  方向      */
    tmc_run_mode_t run_mode;            /* 运行模式 */
    uint8_t enable;                     // 使能状态
    uint32_t step_frequency;            // 步进频率(Hz)
    uint8_t stealth_enabled;   /* StealthChop使能 */
    uint8_t spread_enabled;    /* SpreadCycle使能 */
    uint32_t tpowerdown;       /* 掉电延时 */
    uint32_t tpwmthrs;         /* TPWMTHRS阈值 */
    uint32_t tcoolthrs;        /* TCOOLTHRS阈值 */
    uint32_t thigh;            /* THIGH阈值 */
} tmc2209_config_t;

/* 电机状态结构体 */
typedef struct {
	uint8_t channelNo;					/* 当前通道号*/
    uint8_t enabled;            /* 使能状态 */
    uint8_t moving;             /* 运动状态 */
    uint8_t homing;             /* 归零状态 */
    uint8_t error;              /* 错误状态 */
    motor_direction_t direction; /* 当前方向 */
    uint32_t position;          /* 当前位置（步） */
    uint32_t target_position;   /* 目标位置 */
    uint32_t speed;             /* 当前速度（Hz） */
    uint32_t target_speed;      /* 目标速度（Hz） */
    //pin_operation_mode_t mode;  /* 操作模式 */
    uint8_t microsteps;         /* 当前微步设置 */
    uint8_t run_mode;           /* 运行模式：0=StealthChop, 1=SpreadCycle */
} motor_status_t;
extern motor_status_t motor_status;
/* 引脚状态结构体 */
typedef struct {
    uint8_t org_state;    /* 归零开关状态 */
    uint8_t diag_state;   /* 诊断引脚状态 */
    uint8_t index_state;  /* 索引信号状态 */
    uint8_t spread_state; /* 模式引脚状态 */
    uint8_t step_state;   /* 步进脉冲状态 */
    uint8_t dir_state;    /* 方向引脚状态 */
    uint8_t en_state;     /* 使能引脚状态 */
} tmc_pin_states_t;

extern bool g_origin_location_flag;

/* UART通信参数 */
#define TMC2209_UART_ADDR    0x03  /* 默认从机地址 */
#define TMC2209_UART_TIMEOUT 3000  /* 超时时间(ms) */


/* TMC2209寄存器地址 */
typedef enum {
    /* 通用配置寄存器 */
    TMC2209_REG_GCONF      = 0x00,
    TMC2209_REG_GSTAT      = 0x01,
    TMC2209_REG_IFCNT      = 0x02,
    TMC2209_REG_SLAVECONF  = 0x03,
    TMC2209_REG_OTP_PROG   = 0x04,
    TMC2209_REG_OTP_READ   = 0x05,
    TMC2209_REG_IOIN       = 0x06,
    TMC2209_REG_FACTORY_CONF = 0x07,
    
    /* 速度和电流寄存器 */
    TMC2209_REG_IHOLD_IRUN = 0x10,
    TMC2209_REG_TPOWERDOWN = 0x11,
    TMC2209_REG_TSTEP      = 0x12,
    TMC2209_REG_TPWMTHRS   = 0x13,
    TMC2209_REG_TCOOLTHRS  = 0x14,
    TMC2209_REG_THIGH      = 0x15,
    
    /* 直接控制寄存器 */
    TMC2209_REG_XDIRECT    = 0x2D,
    TMC2209_REG_VDCMIN     = 0x33,
    
    TMC2209_WREG_SGTHRS    = 0x40,
    TMC2209_RREG_SGRESULT  = 0x41,
    /* 微步配置寄存器 */
    TMC2209_RREG_MSCNT     = 0x6A,
    TMC2209_RREG_MSCURACT  = 0x6B,
    TMC2209_REG_CHOPCONF   = 0x6C,
    TMC2209_REG_COOLCONF   = 0x6D,
    TMC2209_REG_DCCTRL     = 0x6E,
    TMC2209_REG_DRV_STATUS = 0x6F,
    
    /* PWM配置寄存器 */
    TMC2209_REG_PWMCONF    = 0x70,
    TMC2209_REG_PWM_SCALE  = 0x71,
    TMC2209_REG_PWM_AUTO   = 0x72,
} tmc2209_reg_t;

// 步进电机加减速结构体
typedef struct {
    int32_t total_steps;        // 总步数
    int32_t current_step;       // 当前步数
    uint32_t current_speed;     // 当前速度 (PPS)
    uint32_t max_speed;         // 最大速度 (PPS)
    uint32_t acceleration;      // 加速度 (PPS)
    uint8_t state;              // 当前状态: 0=停止, 1=加速, 2=匀速, 3=减速
    uint32_t accel_steps;       // 加速步数
    uint32_t decel_steps;       // 减速步数
    uint32_t accel_end_step;    // 加速结束步数
    uint32_t decel_start_step;  // 减速开始步数
} StepperRamp;
extern StepperRamp stepper;
/* 电机状态 */
typedef enum {
    MOTOR_STOPPED = 0,
    MOTOR_RUNNING,
    MOTOR_HOMING,
    MOTOR_ERROR
} motor_state_t;


/* 通信状态 */
typedef enum {
    UART_OK = 0,
    UART_CRC_ERROR,
    UART_TIMEOUT_ERROR,
    UART_FRAME_ERROR,
    UART_NO_RESPONSE
} uart_status_t;

/* UART帧结构 */
typedef struct {
    uint8_t sync_byte;     /* 同步字节：0x05 */
    uint8_t slave_addr;    /* 从机地址 */
    uint8_t reg_addr;      /* 寄存器地址 */
    uint8_t data[4];       /* 数据字节 */
    uint8_t crc;           /* CRC校验 */
} tmc_uart_frame_t;

// 位置点定义
typedef enum {
    POINT_HOME = 0,  // 归零点
    POINT_P1 = 1,
    POINT_P2,
    POINT_P3,
    POINT_P4,
    POINT_P5,
    POINT_P6,
    POINT_P7,
    POINT_P8,
    POINT_P9,
    POINT_P10,
    POINT_P11,
    POINT_P12,
    POINT_P13,
    POINT_P14,
    POINT_P15,
    POINT_P16,
    POINT_P17,
    POINT_P18,
    POINT_P19,
    POINT_P20,
    POINT_P21,
    POINT_P22,
    POINT_P23,
    POINT_P24,
    MAX_POINTS = 48
} PositionPoint;

// 位置点数据结构
typedef struct {
    uint32_t step_count;      // 对应的步数
    bool calibrated;          // 是否已校准
    uint32_t calibration_time;// 校准时间戳
} PositionPointData;

// 位置点配置结构体
typedef struct {
    PositionPointData points[MAX_POINTS];  // 所有位置点数据
    uint32_t version;       // 数据版本
    uint32_t max_steps_num; //设置最大步数值
    uint32_t last_save_time;// 最后保存时间,20 26 01 15 17 26 bcd码 
    uint32_t crc32;         // 数据校验码
} PositionConfig;

// Flash存储配置
#define CONFIG_FLASH_ADDR    0x000000  // 存储在Flash的起始地址
#define CONFIG_MAGIC_NUMBER  0x504F494E  // "POIN"的ASCII码
// 错误码
typedef enum {
    DRV_OK = 0,
    DRV_ERR_INVALID_POINT,
    DRV_ERR_MOTOR_BUSY,
    DRV_ERR_MOVE_FAILED,
    DRV_ERR_FLASH_READ,
    DRV_ERR_FLASH_WRITE,
    DRV_ERR_CONFIG_CORRUPT,
    DRV_ERR_HOMING_TIMEOUT,
} DriverError;

extern bool motor_busy ;
extern PositionConfig position_config;
//extern volatile uint32_t system_tick;
extern volatile uint8_t wait_for_compensation_cnt;


static DriverError load_config_from_flash(void);
static void init_default_config(void);
DriverError save_config_to_flash(void);



/* 函数声明 */
void tmc2209_init(void);
void tmc2209_set_direction(motor_direction_t dir);
void tmc2209_set_current(uint16_t current_ma, uint8_t hold_percent);
void tmc2209_set_run_mode(tmc_run_mode_t mode);
void tmc2209_step_pulse(void);

DriverError tmc2209_homing(uint32_t speed_hz);
void tmc2209_print_pin_states(void);




void StepperMotor_Init(StepperMotor_t *motor, uint16_t full_steps_per_rev, uint16_t microsteps);
float StepsToDegrees(StepperMotor_t *motor, int32_t steps);
int32_t DegreesToSteps(StepperMotor_t *motor, float angle);
void SetMotorZeroPosition(StepperMotor_t *motor);
void UpdateMotorSteps(StepperMotor_t *motor, uint8_t dir);
float GetCurrentAngle(StepperMotor_t *motor);
int32_t StepsToAngleWithDirection(StepperMotor_t *motor, int32_t steps, float *angle);
uint32_t TMC2209_ReadRegister(uint8_t reg_addr);
void TMC2209_WriteRegister(uint8_t reg_addr, uint32_t data);
void TMC2209_Config_800Steps_90Degrees(void);
void tmc2209_stallguard_config(void);


int32_t get_point_steps(PositionPoint point);

// 电机移动到绝对步数位置
DriverError move_to_absolute_step(PositionPoint point,int32_t targetstep);

// 电机移动到指定位置点
//DriverError move_to_point(PositionPoint point);
DriverError move_to_point(PositionPoint point);
// 电机归零
DriverError move_to_home(void);

// 检查电机是否忙
bool is_motor_busy(void);

// 设置电机速度（步/秒）
void set_motor_speed(uint32_t steps_per_sec);

void tmc2209_stall_detected(void);

void Save_position_config(uint32_t addr,uint8_t *par);
    
int get_calibrated_point_count(void);
bool is_point_calibrated(PositionPoint point);
DriverError clear_calibration(void);
DriverError save_position_config(void) ;
void tmc2209_move_steps(uint32_t steps, uint32_t frequency_hz, motor_direction_t dir);
void tmc2209_move_steps_ramp(uint32_t steps, uint32_t frequency_hz, motor_direction_t dir);
void tmc2209_timer_set_frequency(uint32_t frequency_hz);
void wait_for_compensation(void);

void tmc2209_disable_motor(void);
void tmc2209_calculate_deviation(void);
#endif /* __TMC2209_H */
