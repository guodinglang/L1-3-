/****************************************************************************************
*
*	模块名称 : TMC2209
*	文件名称 : 
*	版    本 : V0.1
*	说    明 : 马达驱动芯片tmc2209通信服务函数
*	修改记录 :
*   版 本 号 :  
*   日    期 :  2026-6-17   
*   作    者 :  G-D-L 
*	说    明 :
*
*****************************************************************************************/
#include "project.h"
#include "tmc_2209_comm.h"
volatile uint32_t system_tick = 0; // 系统时间戳

static void delay_us_nop(uint32_t us)
{
	uint32_t i;
    for(i = 0; i < us; i++) {
       __NOP();
    }
}

/***************************************************************************
;** 函数名称: 	swuart_calcCRC
;** 功能描述:   软件CRC计算
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	软件CRC计算，当MCU没有硬件crc计算单元时，可以使用此函数进行CRC校验
;***************************************************************************/
void swuart_calcCRC(uint8_t* datagram, uint8_t datagramLength)
{
    int i,j;
    uint8_t* crc = datagram + (datagramLength-1); // CRC located in last byte of message
    uint8_t currentByte;
    *crc = 0;
    for (i=0; i<(datagramLength-1); i++) 
    { // Execute for all bytes of a message
        currentByte = datagram[i]; // Retrieve a byte to be sent from Array
        for (j=0; j<8; j++) 
        {
            if ((*crc >> 7) ^ (currentByte&0x01)) // update CRC based result of XOR operation
            {
                *crc = (*crc << 1) ^ 0x07;
            }
            else
            {
                *crc = (*crc << 1);
            }
            currentByte = currentByte >> 1;
        } // for CRC bit
    } // for message byte
}

/***************************************************************************
;** 函数名称: 	calculate_crc
;** 功能描述:   软件CRC计算
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	硬件CRC计算
;***************************************************************************/
uint8_t calculate_crc(uint8_t *data, uint8_t length)
{
    uint8_t crc = 0;
    uint8_t currentByte;
	uint8_t i,j;
    for (i = 0; i < length; i++) {
        //crc ^= data[i];
        currentByte = data[i];
        for (j = 0; j < 8; j++) {
            if ((crc >> 7) ^ (currentByte&0x01)) // update CRC based result of XOR operation
            {
                crc = (crc << 1) ^ 0x07;
            }
            else
            {
                crc = (crc << 1);
            }
            currentByte = currentByte >> 1;
        }
    }
    return crc;
}

/***************************************************************************
;** 函数名称: 	uart_send_byte
;** 功能描述:   发送一个字节
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
static void uart_send_byte(uint8_t data)
{
    /* 等待发送缓冲区空 */
    while(usart_flag_get(TMC_UART, USART_TDBE_FLAG) == RESET);
    
    /* 等待发送缓冲区空 */
    usart_data_transmit(TMC_UART, data);
    
/* 全局引脚状态 */
    while(usart_flag_get(TMC_UART, USART_TDC_FLAG) == RESET);
}
/***************************************************************************
;** 函数名称: 	send_uart_frame
;** 功能描述:   发送UART帧
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
static void send_uart_frame(uint8_t addr, uint8_t reg, uint32_t data, uint8_t is_write)
{
    uint8_t tx_data[8];
    uint8_t i;
    
    /* 等待发送缓冲区空 */
    tx_data[0] = 0x05;               /* 同步字节 */
    tx_data[1] = addr;               /* 从机地址 */
    tx_data[2] = reg;                /* 寄存器地址 */
    
    if(is_write) {
        tx_data[2] |= 0x80;          /* 写命令 */
        tx_data[3] = (data >> 24) & 0xFF;
        tx_data[4] = (data >> 16) & 0xFF;
        tx_data[5] = (data >> 8) & 0xFF;
        tx_data[6] = data & 0xFF;
        tx_data[7] = calculate_crc(&tx_data[0], 7);
    } else {
        tx_data[3] = calculate_crc(&tx_data[0], 3);
    }
    if (is_write)
    {
        uart_SendRWFlag=0;
        /* 发送数据 */
        for(i = 0; i < 8; i++) {
            uart_send_byte(tx_data[i]);
        }
    }
    else
    {
        uart_SendRWFlag=1;
        /* 发送数据 */
        for(i = 0; i < 4; i++) {
            uart_send_byte(tx_data[i]);
        }
    }
}

/***************************************************************************
;** 函数名称: 	receive_uart_frame
;** 功能描述:   接收UART帧
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
static uart_status_t receive_uart_frame(uint8_t *addr, uint8_t *reg, uint32_t *data)
{
    uint8_t rx_data[8];
//    uint8_t sync_byte;
    uint8_t crc_calc, crc_recv;
    
/* 全局引脚状态 */
    uint32_t timeout = system_tick + TMC2209_UART_TIMEOUT;
    while(!uart_rx_complete) {
        if(system_tick > timeout) {
            return UART_TIMEOUT_ERROR;
        }
    }
    if (uart_rx_index>=12)
    {
        memcpy(rx_data, &uart_rx_buffer[4], 8);
    }
    else
    {
        /* 发送数据 */
        memcpy(rx_data, uart_rx_buffer, 8);
    }
    uart_rx_index = 0;
    uart_rx_complete = 0;
    
/* 全局引脚状态 */
    if(rx_data[0] != 0x05) {
        return UART_FRAME_ERROR;
    }
    
    /* 验证CRC */
    crc_calc = calculate_crc(&rx_data[0],7);
    crc_recv = rx_data[7];
    
    if(crc_calc != crc_recv) {
        return UART_CRC_ERROR;
    }
    
    /* 等待发送缓冲区空 */
    *addr = rx_data[1];
    *reg = rx_data[2] & 0x7F;
    *data = ((uint32_t)rx_data[3] << 24) |
            ((uint32_t)rx_data[4] << 16) |
            ((uint32_t)rx_data[5] << 8) |
            ((uint32_t)rx_data[6]);
    
    return UART_OK;
}

/***************************************************************************
;** 函数名称: 	tmc2209_uart_read_reg
;** 功能描述:   读取寄存器值
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
uart_status_t tmc2209_uart_read_reg(tmc2209_reg_t reg, uint32_t *data)
{
    uint8_t addr, reg_addr;
    uint32_t reg_data;
	uart_status_t status;
    
    /* 等待发送缓冲区空 */
    send_uart_frame(tmc2209_config.slave_address, reg, 0, 0);
    //delay_us_nop(500);
    /* 等待发送缓冲区空 */
    status = receive_uart_frame(&addr, &reg_addr, &reg_data);
    
    if(status == UART_OK) 
   {
        if( reg_addr == reg) 
        {
            *data = reg_data;
//            sprintfx("Read register 0x%02X: 0x%08X\r\n", reg, reg_data);
            return UART_OK;
        } else 
        {
//            sprintfx("Read register mismatch: addr=0x%02X, reg=0x%02X\r\n", addr, reg_addr);
            return UART_FRAME_ERROR;
        }
    }
    
    return status;
}

/***************************************************************************
;** 函数名称: 	send_uart_command   
;** 功能描述:   发送UART命令
;** 输入参数:   
;** 返 回 值: 
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   G-D-L
;** 日　  期:   2026-6-16
;** 修改原因：  
;** 说    明:  	
;***************************************************************************/
void send_uart_command(uint8_t address, uint8_t reg, uint8_t *data, uint8_t data_len)
{
    uint8_t command[32];
    uint8_t command_len = 0;
    uint8_t i,crc;

    command[command_len++] = 0x05; // 同步码
    command[command_len++] = (address << 4); // 从机地址
    command[command_len++] = reg; // 寄存器地址

    for (i = 0; i < data_len; i++) {
        command[command_len++] = data[i];
    }

    crc = calculate_crc(command, command_len);
    command[command_len++] = crc;
    uart_SendRWFlag=0;
        /* 发送数据 */
    for(i = 0; i < command_len; i++) {
        usart_data_transmit(TMC_UART, command[i]);
        while(usart_flag_get(TMC_UART, USART_TDBE_FLAG) == RESET);
    }

//    usart_data_transmit(USART3, command, command_len);
//    while (usart_flag_get(USART3, USART_TDC_FLAG) == RESET);
}
/***************************************************************************
;** 函数名称:  receive_uart_response
;** 功能描述:  接收UART响应数据
;** 输    入: 	buffer: 接收缓冲区指针
;           length: 期望接收长度
;** 输    入: 	buffer: 接收缓冲区指针
;** 返 回 值: 	无
;** 全局变量: 
;** 调用模块: usart_flag_get, usart_data_receive
;** 作    者:   G-D-L
;** 日    期:   2026-6-12
;** 修改原因
;** 说    明:    通过UART接收指定长度的数据
;***************************************************************************/
void receive_uart_response(uint8_t *buffer, uint16_t length)
{
    uint16_t i;
    for (i = 0; i < length; i++) {
        while (usart_flag_get(USART3, USART_RDBF_FLAG) == RESET);
        buffer[i] = usart_data_receive(USART3);
    }
}
/***************************************************************************
** 函数名称:  tmc2209_uart_write_reg
** 功能描述:  写入TMC2209寄存器
** 输    入: 	reg: 寄存器地址
**           data: 写入的32位数据
** 输    出: 	无
** 返 回 值: 	通信状态 (UART_OK, UART_TIMEOUT_ERROR, UART_FRAME_ERROR, UART_CRC_ERROR)
** 全局变量: 
** 调用模块: send_uart_frame, receive_uart_frame, delay_us_nop
** 作    者:   G-D-L
** 日    期:   2026-6-12
** 修改原因
** 说    明:    通过UART写入TMC2209指定寄存器，并验证回读值
;***************************************************************************/
uart_status_t tmc2209_uart_write_reg(tmc2209_reg_t reg, uint32_t data)
{
    uint8_t addr, reg_addr;
    uint32_t readback;
	uart_status_t status;
    
    /* 等待发送缓冲区空 */
    send_uart_frame(tmc2209_config.slave_address, reg, data, 1);
    
    /* 等待发送缓冲区空 */
    delay_us_nop(100);
     /* 接收响应 */
    status = receive_uart_frame(&addr, &reg_addr, &readback);
    
    
    if(status == UART_OK) {
        if(readback == data) 
        {
//            sprintfx("Write register 0x%02X: 0x%08X OK\n", reg, data);
            return UART_OK;
        } else 
        {
//            sprintfx("Write register 0x%02X failed: wrote 0x%08X, readback 0x%08X\n", reg, data, readback);
            return UART_FRAME_ERROR;
        }
    }
    
    return status;
}

/***************************************************************************
** 函数名称:  tmc2209_read_status
** 功能描述:  读取驱动器状态
** 输    出: 	无
** 输    出: 	无
** 返 回 值: 	驱动器状态结构体
** 全局变量: 
** 调用模块: tmc2209_uart_send_command
** 作    者:   G-D-L
** 日    期:   2026-6-12
** 修改原因
** 说    明:    读取TMC2209的DRV_STATUS寄存器
;***************************************************************************/
tmc2209_status_t tmc2209_read_status(void)
{
    tmc2209_status_t status = {0};
    uint8_t uart_data[8];
    
    /* 等待发送缓冲区空 */
    uart_data[0] = 0x05;  // 从机地址
    uart_data[1] = 0x03; 
    uart_data[2] = 0x6F;  // DRV_STATUS寄存器地址
    uart_data[3] = 0x00;  // CRC
    
    tmc2209_uart_send_command(uart_data, 4);
    
    /* 等待发送缓冲区空 */

    
    return status;
}
/***************************************************************************
** 函数名称:  TMC2209_UART_Send
** 功能描述:  发送UART数据
** 输    入: 	data: 要发送的数据指针
**           len: 数据长度
** 输    出: 	无
** 返 回 值: 	驱动器状态结构体
** 全局变量: 
** 调用模块: usart_flag_get, usart_data_transmit
** 作    者:   G-D-L
** 日    期:   2026-6-12
** 修改原因
** 说    明:    通过UART发送指定长度的数据
;***************************************************************************/
void TMC2209_UART_Send(uint8_t *data, uint8_t len)
{
	uint8_t i;
    for (i = 0; i < len; i++) {

        while(usart_flag_get(TMC_UART, USART_TDBE_FLAG) == RESET);
        usart_data_transmit(TMC_UART, data[i]);
    }
}
/***************************************************************************
** 函数名称:  TMC2209_UART_Receive
** 功能描述:  发送UART数据
** 输    入: 	data: 要发送的数据指针
**           len: 数据长度
** 输    入: 	data: 要发送的数据指针
** 返 回 值: 	驱动器状态结构体
** 全局变量: 
** 调用模块: usart_flag_get, usart_data_receive
** 作    者:   G-D-L
** 日    期:   2026-6-12
** 修改原因
** 说    明:    通过UART发送指定长度的数据
;***************************************************************************/
uint8_t TMC2209_UART_Receive(uint8_t *data, uint8_t len) {
    uint8_t received = 0;
    uint32_t timeout = 1000000;  // 超时计数
    
    while (received < len && timeout--) {
        if (usart_flag_get(TMC_UART, USART_RDBF_FLAG) != RESET) {
            data[received++] = usart_data_receive(TMC_UART);
        }
    }
    return received;
}
/***************************************************************************
** 函数名称:  TMC2209_WriteRegister
** 功能描述:  写入TMC2209寄存器
** 输    入: 	reg_addr: 寄存器地址
**           data: 32位数据
** 输    出: 	无
** 返 回 值: 	驱动器状态结构体
** 全局变量: 
** 调用模块: TMC2209_UART_Send, calculate_crc, delay_ms
** 作    者:   G-D-L
** 日    期:   2026-6-12
** 修改原因
** 说    明:    构造TMC2209协议帧并写入指定寄存器
;***************************************************************************/
void TMC2209_WriteRegister(uint8_t reg_addr, uint32_t data) {
    uint8_t tx_buffer[8];
    

    tx_buffer[0] = 0x05;                     // 同步字节
    tx_buffer[1] = TMC2209_SLAVE_ADDR;      // 从机地址
    tx_buffer[2] = reg_addr | TMC2209_WRITE_BIT;  // 寄存器地址 + 写标志
    tx_buffer[3] = (data >> 24) & 0xFF;     // 数据高位
    tx_buffer[4] = (data >> 16) & 0xFF;
    tx_buffer[5] = (data >> 8) & 0xFF;
    tx_buffer[6] = data & 0xFF;             // 数据低位
    tx_buffer[7] = calculate_crc(tx_buffer, 7);  // CRC校验


    TMC2209_UART_Send(tx_buffer, 8);
    

    delay_ms(1);
}
/***************************************************************************
** 函数名称:  TMC2209_ReadRegister
** 功能描述:  写入TMC2209寄存器
** 输    入: 	reg_addr: 寄存器地址
** 输    出: 	无
** 返 回 值: 	寄存器值(失败返回0xFFFFFFFF)
** 全局变量: 
** 调用模块: TMC2209_UART_Send, TMC2209_UART_Receive, calculate_crc
** 作    者:   G-D-L
** 日    期:   2026-6-12
** 修改原因
** 说    明:    发送读请求并解析响应，验证CRC
;***************************************************************************/
uint32_t TMC2209_ReadRegister(uint8_t reg_addr) {
    uint8_t tx_buffer[4];
    uint8_t rx_buffer[12] = {0};
	uint8_t received;
    

    tx_buffer[0] = 0x05;                     // 同步字节
    tx_buffer[1] = TMC2209_SLAVE_ADDR;      // 从机地址
    tx_buffer[2] = reg_addr | TMC2209_READ_BIT;  // 寄存器地址 + 读标志
    tx_buffer[3] = calculate_crc(tx_buffer, 3);  // CRC校验
    
    TMC2209_UART_Send(tx_buffer, 4);
    

    received = TMC2209_UART_Receive(rx_buffer, 12);
    
    if (received >= 12) {

        if (rx_buffer[7] == calculate_crc(rx_buffer, 7)) {
            return (rx_buffer[3] << 24) | (rx_buffer[4] << 16) | 
                   (rx_buffer[5] << 8) | rx_buffer[6];
        }
    }
    
    return 0xFFFFFFFF;  // 读取失败
}
/***************************************************************************
** 函数名称:   tmc2209_uart_send_command
** 功能描述:   发送UART数据
** 输    入:   data: 要发送的数据指针
**             len: 数据长度
** 输    出:   无
** 返 回 值:   驱动器状态结构体
** 全局变量: 
** 调用模块:   usart_flag_get, usart_data_transmit
** 作    者:   G-D-L
** 日    期:   2026-6-12
** 修改原因
** 说    明:   通过UART发送指定长度的数据
;***************************************************************************/
void tmc2209_uart_send_command(uint8_t *data, uint8_t len)
{
	uint8_t i;
    /* 使能UART时钟 */
    while(usart_flag_get(TMC_UART, USART_TDBE_FLAG) == RESET);
    
    /* 等待发送缓冲区空 */
    for(i = 0; i < len; i++) {
        usart_data_transmit(TMC_UART, data[i]);
        while(usart_flag_get(TMC_UART, USART_TDBE_FLAG) == RESET);
    }
}
/***************************************************************************
** 函数名称:   TMC_UART_IRQHANDLER
** 功能描述:   tmc2209 UART中断服务函数
** 输    出:   无
** 输    出:   无
** 返 回 值:   驱动器状态结构体
** 全局变量: 
** 调用模块:   usart_flag_get, usart_data_receive, usart_flag_clear
** 作    者:   G-D-L
** 日    期:   2026-6-12
** 修改原因:
** 说    明:
;***************************************************************************/
void TMC_UART_IRQHANDLER(void)
{
    if(usart_flag_get(TMC_UART, USART_RDBF_FLAG) != RESET) {
        uint8_t data = (uint8_t)usart_data_receive(TMC_UART);
        
        /* 发送数据 */
        if(uart_rx_index < sizeof(uart_rx_buffer)) {
            uart_rx_buffer[uart_rx_index++] = data;
            
/* 全局引脚状态 */
            if(uart_rx_index >= uart_SendRWFlag*4+8) {  /* TMC2209响应帧为8字节 */
                uart_rx_complete = 1;
            }
        } else {
/* 全局引脚状态 */
            uart_rx_index = 0;
        }
        usart_flag_clear(USART3,USART_RDBF_FLAG);
    }
}

