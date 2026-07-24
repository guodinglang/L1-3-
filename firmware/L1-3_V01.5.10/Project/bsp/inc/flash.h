/**
  **************************************************************************
  * @file     flash.h
  * @version  v2.0.5
  * @date     2022-05-20
  * @brief    flash header file
  **************************************************************************
  *                       Copyright notice & Disclaimer
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_H__
#define __FLASH_H__


#include "project.h"

//-----------------------------------------------------------------------------------------------
#define SECTOR_SIZE              1024   //AT32F415KBU7-4 128Kbyte flash, 128个扇区，每个扇区1K
//#define FLASH_ADDRESS_START      (0x08000000 + 1024 * 120) //from 120 section start
//#define FLASH_SECTION(n)         (FLASH_BASE + SECTOR_SIZE * (120+(n)))	//Save section start

#define FLASH_SECTION_PARAMETER   (FLASH_BASE + SECTOR_SIZE * 121)
#define IDN_ADDR                  10 	//10~34   24byte
#define PN_ADDR                   40 	//40~56   16byte
#define SN_ADDR                   60 	//60~76   16byte
#define NCH_ADDR                  80    //70      1byte

#define OFS_ADDR                  90    //90--97  	8byte
#define OFL_ADDR                  100   //100--107  8byte

#define VER_ADDR					160		//24 byte
#define PPS_ADDR					184		//1 byte

#define FLASH_SECTION_INTERIM	  (FLASH_BASE + SECTOR_SIZE * 122)
#define FLASH_SECTION_LEVEL1      (FLASH_BASE + SECTOR_SIZE * 123)
#define FLASH_SECTION_LEVEL2      (FLASH_BASE + SECTOR_SIZE * 124)
#define FLASH_SECTION_LEVEL3      (FLASH_BASE + SECTOR_SIZE * 125)
#define FLASH_SECTION_LEVEL4      (FLASH_BASE + SECTOR_SIZE * 126)


//-----------------------------------------------------------------------------------------------
void flash_read_byte(uint32_t addr, uint8_t *pd, uint8_t num);
void flash_read_halfword(uint32_t read_addr, uint16_t *p_buffer, uint16_t num_read);
void flash_write_nocheck(uint32_t write_addr, uint16_t *p_buffer, uint16_t num_write);
//void flash_write(uint32_t write_addr,uint16_t *p_Buffer, uint16_t num_write);
void flash_read_word(uint32_t addr, uint32_t *pb, uint16_t num);
void SaveParamet(void);
void flash_read_level(uint16_t ch,uint16_t *rd);
void ReadParamet(uint32_t addr,void *wd);





#endif

















