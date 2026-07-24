#include "project.h"
//#include "main.h"

/**
 * @brief  主程序入口函数
 * @note   系统初始化和主要功能配置
 * @param  无
 * @retval int: 返回状态码
 */
int main(void)
{
	system_clock_config(); // 初始化系统时钟 240MHz
	SysTickConfigure();    // 配置系统滴答定时器 1ms
	IoConfig();	           // 配置输入输出引脚
	Time3_Init();          // PWM for MOTOR  M_STEP
	usart_configuration(115200);  // 配置USART1 115200 8 1 0
//	SPI1_Config();                // 配置SPI1 连接AD5504 本项目不使用
	W25Q128_SPI2Config();         // 配置SPI2 连接W25Q128
	W25Q128_ParametInit();        // 从W25Q128读取参数到RAM
	InterADconfig();              // 配置内部ADC
	bsp_Init();                   // 初始化外设

	/* 初始化TMC2209 */
    tmc2209_init();               // 初始化TMC2209步进电机驱动器
    tmc2209_stallguard_config();  // 配置TMC2209 stall guard

	while(1)
	{
		InputIOScan();            // 读取D0~D5
		SwitchMessage();          // 按键处理
		rxCommand();              // 解析串口命令
		AcquisitMCP9700A();       // PCB温度采样
		wait_for_compensation();  // 实时纠偏
//		WorkStateLED();
//		wdt_counter_reload();
	}
}



