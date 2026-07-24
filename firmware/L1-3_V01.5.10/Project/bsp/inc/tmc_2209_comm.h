/**
 * @file tmc_2209_comm.h
 * @brief TMC2209 commication 驱动程序头文件
 * @version 1.0
 */

#ifndef __TMC_2209_COMM_H
#define __TMC_2209_COMM_H

#include "at32f403a_407.h"
#include "at32f403a_407_gpio.h"
#include <stdbool.h>

static uint8_t uart_rx_buffer[64]; // UART接收缓冲区
static uint8_t uart_rx_index = 0;  // UART接收缓冲区索引
static volatile uint8_t uart_rx_complete = 0; // UART接收完成标志
static volatile uint8_t uart_SendRWFlag=0; //写0，读1
extern volatile uint32_t system_tick; // 系统时间戳
extern tmc2209_config_t tmc2209_config;

void swuart_calcCRC(uint8_t* datagram, uint8_t datagramLength);
uint8_t calculate_crc(uint8_t *data, uint8_t length);
uart_status_t tmc2209_uart_read_reg(tmc2209_reg_t reg, uint32_t *data);

void send_uart_command(uint8_t address, uint8_t reg, uint8_t *data, uint8_t data_len);
void receive_uart_response(uint8_t *buffer, uint16_t length);
uart_status_t tmc2209_uart_write_reg(tmc2209_reg_t reg, uint32_t data);
tmc2209_status_t tmc2209_read_status(void);
void tmc2209_uart_send_command(uint8_t *data, uint8_t len);
void TMC2209_UART_Send(uint8_t *data, uint8_t len);
uint8_t TMC2209_UART_Receive(uint8_t *data, uint8_t len);
uint32_t TMC2209_ReadRegister(uint8_t reg_addr);
#endif

