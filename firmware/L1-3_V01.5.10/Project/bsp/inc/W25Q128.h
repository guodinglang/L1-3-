#ifndef __EXFLASH_FUNC_H__
#define __EXFLASH_FUNC_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* macro */
/* includes ------------------------------------------------------------------*/
#define SPI_SEL                         SPI2
#define SPI_PERIPH_CLOCK                CRM_SPI2_PERIPH_CLOCK
#define SPI_GPIO_PORT                   GPIOB
#define SPI_GPIO_CLOCK                  CRM_GPIOB_PERIPH_CLOCK

#define SPI_NSS_PIN                     GPIO_PINS_12
#define SPI_SCK_PIN                     GPIO_PINS_13
#define SPI_MISO_PIN                    GPIO_PINS_14
#define SPI_MOSI_PIN                    GPIO_PINS_15

#define SPI_CS_L()                      (SPI_GPIO_PORT->clr = SPI_NSS_PIN)
#define SPI_CS_H()                      (SPI_GPIO_PORT->scr = SPI_NSS_PIN)

/* w25Q128 para */
#define EXFLASH_ID                      (0xEF17)
#define EXFLASH_CHIP_SEC_NUM            (4096)
#define EXFLASH_SEC_SIZE                (4096) // secotr size
#define EXFLASH_PAGE_SIZE               (256)  // page size

// cmd
#define CMD_FLASH_WR_EN                 (0x06)
#define CMD_FLASH_WR_DIS                (0x04)

#define CMD_FLASH_READ_STA_1            (0x05)
#define CMD_FLASH_READ_STA_2            (0x35)
#define CMD_FLASH_READ_STA_3            (0x15)
#define CMD_FLASH_WEITE_STA_1           (0x01)
#define CMD_FLASH_WEITE_STA_2           (0x31)
#define CMD_FLASH_WEITE_STA_3           (0x11)

#define CMD_FLASH_MODE_ADD_4BYTE        (0xB7)    //Enter 4-Byte Address Mode

#define CMD_FLASH_READ_DATA             (0x03)
#define CMD_FLASH_PAGE_PROGRAM          (0x02)
#define CMD_FLASH_SECTOR_ERASE          (0x20)
#define CMD_FLASH_CHIP_ERASE            (0xC7)
#define CMD_FLASH_DEVICE_ID             (0x90)
#define CMD_FLASH_FAST_READ             (0x0B)    //Fast Read
#define CMD_FLASH_BLOCK_ERASE           (0xD8)    //Block Erase (64KB)

/********************/
#define SPIF_CHIP_SIZE                   (0x1000000)
#define SPIF_SECTOR_SIZE                 (4096)
#define SPIF_PAGE_SIZE                   (256)

#define SPIF_WRITEENABLE                 (0x06)
#define SPIF_WRITEDISABLE                (0x04)
/* s7-s0 */
#define SPIF_READSTATUSREG1              (0x05)
#define SPIF_WRITESTATUSREG1             (0x01)
/* s15-s8 */
#define SPIF_READSTATUSREG2              (0x35)
#define SPIF_WRITESTATUSREG2             (0x31)
/* s23-s16 */
#define SPIF_READSTATUSREG3              (0x15)
#define SPIF_WRITESTATUSREG3             (0x11)
#define SPIF_READDATA                    (0x03)
#define SPIF_FASTREADDATA                (0x0B)
#define SPIF_FASTREADDUAL                (0x3B)
#define SPIF_PAGEPROGRAM                 (0x02)
/* block size:64kb */
#define SPIF_BLOCKERASE                  (0xD8)
#define SPIF_SECTORERASE                 (0x20)
#define SPIF_CHIPERASE                   (0xC7)
#define SPIF_POWERDOWN                   (0xB9)
#define SPIF_RELEASEPOWERDOWN            (0xAB)
#define SPIF_DEVICEID                    (0xAB)
#define SPIF_MANUFACTDEVICEID            (0x90)
#define SPIF_JEDECDEVICEID               (0x9F)
#define FLASH_SPI_DUMMY_BYTE             (0xA5)

#define MAX_NM                  (96)        // max number
#define MAX_CH                  (16)        // one device max chn

#define FLASH_SECTION_FACTORY       (1*4096) //FLASH_SECTION_FACTORY和FLASH_SECTION_PARAMETER这两个保存一样
#define FLASH_SECTION_PARAMETER     (2*4096)
#define SPIF_SECTION_INTERIM        (3*4096)

#define FLASH_BLOCK_LEVEL           (5*4096)
#define FLASH_CONFIG_ADDR           (6*4096) // 开环马达配置地址
///* para struct */
//typedef struct
//{
//    unsigned char u8InitFlag;     // parameter initialize flag
//    unsigned char u8VerInfor[48]; // version information
//    unsigned char u8IDNInfo[128]; // idn information
//    unsigned char u8MaxNNum;      // max N device number
//    unsigned char u8MaxNChnNum;   // max N chn number
//    unsigned char u8BaudIndex;    // baudrate index
//}PARACtrl_t;

//extern PARACtrl_t xParaCtrl;

typedef struct
{    
    unsigned short int xp;
    unsigned short int xn;
    unsigned short int yp;
    unsigned short int yn;
    unsigned char      ps;
    unsigned short int ch;
    char               nm;
}LEVELcCtrl_t;

extern LEVELcCtrl_t xLevelCtrl;

/* function */
//void vReadParamet(unsigned int addr,unsigned char *par);
//void vSaveParamet(unsigned int addr,unsigned char *par);
//void spiFlash_read_level(void *rd);
void W25Q128_readlevel(void *rd);
void W25Q128_SaveParamet(uint32_t addr,uint8_t *par);
void W25Q128_ReadParamet(uint32_t addr,uint8_t *par);
void W25Q128_SPI2Config(void);

uint8_t W25Q128_LevelSave(void *sd);
unsigned short int spiflash_read_id(void);
void spiflash_write_enable(void);
void spiflash_sector_erase(uint32_t erase_addr);
void spiflash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length);
void spiflash_read(uint8_t *pbuffer, uint32_t read_addr, uint32_t length);
void spiflash_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length);


void W25Q128_ReadMulByte(uint32_t read_addr,uint8_t *pbuffer,uint32_t length);
void W25Q128_WriteMulByte(uint32_t write_addr,uint8_t *pbuffer,  uint32_t length);
void W25Q128_SectorErase(uint32_t erase_addr);
#ifdef __cplusplus
}
#endif

#endif /* __EXFLASH_FUNC_H__ */
