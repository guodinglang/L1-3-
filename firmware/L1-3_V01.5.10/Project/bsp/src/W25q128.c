#include "project.h"
/****************
1.W25Q128存储大小为128M-bit=16MB，可编程位（地址）为Flash_Size=16*1024*1024=16777216 B。
2.W25Q128包含256个块、每个块（64KB）16个扇区（4096个扇区）、每个扇区（4KB）有16页、每一页有256个字节（Byte）。
3.写数据：一次最多写一页不能跨页写入；擦除：可以选择擦除一个扇区（4KB）、擦除半个块（32KB）、擦除一个块（64KB）、擦除整个芯片。
4.Flash 有一个特点，就是可以将 1 写成 0，但是不能将 0 写成 1，要想将 0 写成 1，必须进行擦除操作。如果要改变数据，就需要先擦除后写数据。
5.可以理解为将W25Q128看成一本电子书，这本书有256个章节，每个章节有16个小节，每个小节有16页，每页有256个字。
*******************/
static uint8_t spiflash_sector_buf[SPIF_SECTOR_SIZE] = {0};
//static uint8_t spi_wr_pBuff[SPIF_SECTOR_SIZE] = {0};

////PARACtrl_t  xParaCtrl  = {0};
//LEVELcCtrl_t xLevelCtrl= {0};

/**
*@brief  system clock config program
*@note   the system clock is configured as follow:
*@param  none
*@retval none
*/
void W25Q128_SPI2Config(void)
{
    gpio_init_type gpio_init_struct;
    spi_init_type  spi_init_struct;

    crm_periph_clock_enable(CRM_SPI2_PERIPH_CLOCK, TRUE); /* enable peripheral SPI2 clk */
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK,TRUE); /* enable peripheral GPIOB clk */

    /* initialize gpio */
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_mode             = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins             = SPI_SCK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN;
    gpio_init_struct.gpio_out_type         = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull             = GPIO_PULL_UP;
    gpio_init_struct.gpio_drive_strength   = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(SPI_GPIO_PORT, &gpio_init_struct);

    /* cs pin */
    gpio_init_struct.gpio_mode             = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_pins             = SPI_NSS_PIN;
    gpio_init_struct.gpio_out_type         = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull             = GPIO_PULL_UP;
    gpio_init_struct.gpio_drive_strength   = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(SPI_GPIO_PORT, &gpio_init_struct);
    SPI_CS_H();

    /* initialize spi */
    spi_default_para_init(&spi_init_struct);
    spi_init_struct.transmission_mode      = SPI_TRANSMIT_FULL_DUPLEX;
    spi_init_struct.master_slave_mode      = SPI_MODE_MASTER;
    spi_init_struct.mclk_freq_division     = SPI_MCLK_DIV_8;
    spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
    spi_init_struct.frame_bit_num          = SPI_FRAME_8BIT;
    spi_init_struct.clock_polarity         = SPI_CLOCK_POLARITY_HIGH;
    spi_init_struct.clock_phase            = SPI_CLOCK_PHASE_2EDGE;
    spi_init_struct.cs_mode_selection      = SPI_CS_SOFTWARE_MODE;

    spi_init(SPI_SEL, &spi_init_struct);

    spi_enable(SPI_SEL, TRUE);
}

/*******************************************************************/

/**
  * @brief  write a byte to flash
  * @param  data: data to write
  * @retval flash return data
  */
uint8_t SPI2_WriteOneByte(uint8_t data)
{
    uint8_t brxbuff;
    spi_i2s_dma_transmitter_enable(SPI_SEL, FALSE);
    spi_i2s_dma_receiver_enable(SPI_SEL, FALSE);
    spi_i2s_data_transmit(SPI_SEL, data);
    while(spi_i2s_flag_get(SPI_SEL, SPI_I2S_RDBF_FLAG) == RESET);
    brxbuff = spi_i2s_data_receive(SPI_SEL);
    while(spi_i2s_flag_get(SPI_SEL, SPI_I2S_BF_FLAG) != RESET);
    return brxbuff;
}
/**
  * @brief  write data continuously
  * @param  pbuffer: the pointer for data buffer
  * @param  length: buffer length
  * @retval none
  */
void SPI2_WriteMulByte(uint8_t *pbuffer, uint32_t length)
{
    volatile uint8_t dummy_data;

    while(length--)
    {
        while(spi_i2s_flag_get(SPI_SEL, SPI_I2S_TDBE_FLAG) == RESET);
        spi_i2s_data_transmit(SPI_SEL, *pbuffer);
        while(spi_i2s_flag_get(SPI_SEL, SPI_I2S_RDBF_FLAG) == RESET);
        dummy_data = spi_i2s_data_receive(SPI_SEL);
        pbuffer++;
    }
}
/**
  * @brief  read data continuously
  * @param  pbuffer: buffer to save data
  * @param  length: buffer length
  * @retval none
  */
void SPI2_ReadMulByte(uint8_t *pbuffer, uint32_t length)
{
    uint8_t write_value = FLASH_SPI_DUMMY_BYTE;

    while(length--)
    {
        while(spi_i2s_flag_get(SPI_SEL, SPI_I2S_TDBE_FLAG) == RESET);
        spi_i2s_data_transmit(SPI_SEL, write_value);
        while(spi_i2s_flag_get(SPI_SEL, SPI_I2S_RDBF_FLAG) == RESET);
        *pbuffer = spi_i2s_data_receive(SPI_SEL);
        pbuffer++;
    }
}
/**
  * @brief  read a byte to flash
  * @param  none
  * @retval flash return data
  */
uint8_t SPI2_ReadOneByte(void)
{
    return (SPI2_WriteOneByte(FLASH_SPI_DUMMY_BYTE));
}

/**
  * @brief  enable write operation
  * @param  none
  * @retval none
  */
void SPI2_WriteEnable(void)
{
    SPI_CS_L();
    SPI2_WriteOneByte(SPIF_WRITEENABLE);
    SPI_CS_H();
}

/**
  * @brief  read sr1 register
  * @param  none
  * @retval none
  */
uint8_t SPI2_ReadSr1(void)
{
    uint8_t breadbyte = 0;
    SPI_CS_L();
    SPI2_WriteOneByte(SPIF_READSTATUSREG1);
    breadbyte = (uint8_t)SPI2_ReadOneByte();
    SPI_CS_H();
    return (breadbyte);
}

/**
  * @brief  wait program done
  * @param  none
  * @retval none
  */
void SPI2_WaitBusy(void)
{
    while((SPI2_ReadSr1() & 0x01) == 0x01);
}

/**
  * @brief  erase a sector data
  * @param  erase_addr: sector address to erase
  * @retval none
  */
void W25Q128_SectorErase(uint32_t erase_addr)
{
    SPI2_WriteEnable();
    SPI2_WaitBusy();
    SPI_CS_L();
    SPI2_WriteOneByte(SPIF_SECTORERASE);
    SPI2_WriteOneByte((uint8_t)((erase_addr) >> 16));
    SPI2_WriteOneByte((uint8_t)((erase_addr) >> 8));
    SPI2_WriteOneByte((uint8_t)erase_addr);
    SPI_CS_H();
    SPI2_WaitBusy();
}

/**
  * @brief  read data from flash
  * @param  pbuffer: the pointer for data buffer
  * @param  read_addr: the address where the data is read
  * @param  length: buffer length
  * @retval none
  */
void W25Q128_ReadMulByte(uint32_t read_addr,uint8_t *pbuffer,uint32_t length)
{
    SPI_CS_L();
    SPI2_WriteOneByte(SPIF_READDATA); /* send instruction */
    SPI2_WriteOneByte((uint8_t)((read_addr) >> 16)); /* send 24-bit address */
    SPI2_WriteOneByte((uint8_t)((read_addr) >> 8));
    SPI2_WriteOneByte((uint8_t)read_addr);
    SPI2_ReadMulByte(pbuffer, length);
    SPI_CS_H();
}



/**
  * @brief  write a page data
  * @param  pbuffer: the pointer for data buffer
  * @param  write_addr: the address where the data is written
  * @param  length: buffer length
  * @retval none
  */
void W25Q128_PageWrite(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
    if((0 < length) && (length <= SPIF_PAGE_SIZE))
    {
        SPI2_WriteEnable();            /* set write enable */
        SPI_CS_L();
        SPI2_WriteOneByte(SPIF_PAGEPROGRAM);   /* send instruction */

        /* send 24-bit address */
        SPI2_WriteOneByte((uint8_t)((write_addr) >> 16));
        SPI2_WriteOneByte((uint8_t)((write_addr) >> 8));
        SPI2_WriteOneByte((uint8_t)write_addr);
        SPI2_WriteMulByte(pbuffer,length);
        SPI_CS_H();
        SPI2_WaitBusy();/* wait for program end */
    }
}

/**
  * @brief  write data without check
  * @param  pbuffer: the pointer for data buffer
  * @param  write_addr: the address where the data is written
  * @param  length: buffer length
  * @retval none
  */
void W25Q128_WriteNocheck(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
    uint16_t page_remain;

    /* remain bytes in a page */
    page_remain = SPIF_PAGE_SIZE - write_addr % SPIF_PAGE_SIZE;
    if(length <= page_remain)
    {
        page_remain = length;/* smaller than a page size */
    }
    while(1)
    {
        W25Q128_PageWrite(pbuffer, write_addr, page_remain);
        if(length == page_remain)
        {
            break;/* all data are programmed */
        }
        else
        {
            /* length > page_remain */
            pbuffer += page_remain;
            write_addr += page_remain;
            length -= page_remain;/* the remain bytes to be prorammed */
            if(length > SPIF_PAGE_SIZE)
            {
                page_remain = SPIF_PAGE_SIZE;/* can be progrmmed a page at a time */
            }
            else
            {
                page_remain = length;/* smaller than a page size */
            }
        }
    }
}

/**
  * @brief  write data to flash
  * @param  pbuffer: the pointer for data buffer
  * @param  write_addr: the address where the data is written
  * @param  length: buffer length
  * @retval none
  */
void W25Q128_WriteMulByte(uint32_t write_addr,uint8_t *pbuffer,  uint32_t length)
{
    uint32_t sector_pos;
    uint16_t sector_offset;
    uint16_t sector_remain;
    uint16_t index;
    uint8_t *spiflash_buf;
    spiflash_buf = spiflash_sector_buf;

    /* sector address */
    sector_pos = write_addr / SPIF_SECTOR_SIZE;

    /* address offset in a sector */
    sector_offset = write_addr % SPIF_SECTOR_SIZE;

    /* the remain in a sector */
    sector_remain = SPIF_SECTOR_SIZE - sector_offset;
    if(length <= sector_remain)
    {
        /* smaller than a sector size */
        sector_remain = length;
    }
    while(1)
    {
        /* read a sector */
        W25Q128_ReadMulByte(sector_pos * SPIF_SECTOR_SIZE,spiflash_buf,SPIF_SECTOR_SIZE);

        /* validate the read erea */
        for(index = 0; index < sector_remain; index++)
        {
            if(spiflash_buf[sector_offset + index] != 0xFF)
            {
                /* there are some data not equal 0xff, so this secotr needs erased */
                break;
            }
        }
        if(index < sector_remain)
        {
            /* erase the sector */
            W25Q128_SectorErase(sector_pos* SPIF_SECTOR_SIZE);

            /* copy the write data */
            for(index = 0; index < sector_remain; index++)
            {
                spiflash_buf[index + sector_offset] = pbuffer[index];
            }
            W25Q128_WriteNocheck(spiflash_buf, sector_pos * SPIF_SECTOR_SIZE, SPIF_SECTOR_SIZE); /* program the sector */
        }
        else
        {
            /* write directly in the erased area */
            W25Q128_WriteNocheck(pbuffer, write_addr, sector_remain);
        }
        if(length == sector_remain)
        {
            break;/* write end */
        }
        else
        {
            /* go on writing */
            sector_pos++;
            sector_offset = 0;

            pbuffer += sector_remain;
            write_addr += sector_remain;
            length -= sector_remain;
            if(length > SPIF_SECTOR_SIZE)
            {
                /* could not write the remain data in the next sector */
                sector_remain = SPIF_SECTOR_SIZE;
            }
            else
            {
                /* could write the remain data in the next sector */
                sector_remain = length;
            }
        }
    }
}

uint16_t W25Q128_ReadId(void)
{
    uint16_t wreceivedata = 0;
    SPI_CS_L();
    SPI2_WriteOneByte(SPIF_MANUFACTDEVICEID);
    SPI2_WriteOneByte(0x00);
    SPI2_WriteOneByte(0x00);
    SPI2_WriteOneByte(0x00);
    wreceivedata |= SPI2_ReadOneByte() << 8;
    wreceivedata |= SPI2_ReadOneByte();
    SPI_CS_H();
    return wreceivedata;
}

/***************************************************************************
;** 函数名称:      SaveParamet
;** 功能描述:      Flash Write Section
;** 输入参数:     
;** 返 回 值:     
;** 优 先 级：  
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:   2022-5-26
;** 修改原因：
;***************************************************************************/
void W25Q128_ReadParamet(uint32_t addr,uint8_t *par)
{
    W25Q128_ReadMulByte(addr,par,sizeof(ParameterSturct));
}

/***************************************************************************
;** 函数名称:      SaveParamet
;** 功能描述:      Flash Write Section
;** 输入参数:     
;** 返 回 值:     
;** 优 先 级：  
;** 全局变量: 
;** 调用模块: 
;** 作　  者:   M-L-A
;** 日　  期:   2022-5-26
;** 修改原因：
;***************************************************************************/
void W25Q128_SaveParamet(uint32_t addr,uint8_t *par)
{
    W25Q128_SectorErase(addr);
    W25Q128_WriteMulByte(addr,par,sizeof(ParameterSturct));
}

