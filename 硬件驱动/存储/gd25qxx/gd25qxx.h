#ifndef GD25QXX_H
#define GD25QXX_H

#include "gd32f4xx.h"
#include "gd32f4xx_spi.h"
#include "gd32f4xx_gpio.h"

#define SPI_FLASH_PAGE_SIZE 0x100

/* 使用 GPIOI PIN0 作为 SPI Flash 的 CS 引脚 */
#define SPI_FLASH_CS_LOW()  gpio_bit_reset(GPIOB, GPIO_PIN_12)
#define SPI_FLASH_CS_HIGH() gpio_bit_set  (GPIOB, GPIO_PIN_12)

/* SPI 外设定义，使用 SPI1 */
#define SPI_FLASH          SPI1

#define ARRAYSIZE             (12)

void test_spi_flash(void);

/* initialize SPI0 GPIO and parameter */
void spi_flash_init(void);
/* erase the specified flash sector */
void spi_flash_sector_erase(uint32_t sector_addr);
/* erase the entire flash */
void spi_flash_bulk_erase(void);
/* write more than one byte to the flash */
void spi_flash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint16_t num_byte_to_write);
/* write block of data to the flash */
void spi_flash_buffer_write(uint8_t *pbuffer, uint32_t write_addr, uint16_t num_byte_to_write);
/* read a block of data from the flash */
void spi_flash_buffer_read(uint8_t *pbuffer, uint32_t read_addr, uint16_t num_byte_to_read);
/* read flash identification */
uint32_t spi_flash_read_id(void);
/* initiate a read data byte (read) sequence from the flash */
void spi_flash_start_read_sequence(uint32_t read_addr);
///* read a byte from the SPI flash */
//uint8_t spi_flash_read_byte(void);
///* send a byte through the SPI interface and return the byte received from the SPI bus */
//uint8_t spi_flash_send_byte(uint8_t byte);
///* send a half word through the SPI interface and return the half word received from the SPI bus */
//uint16_t spi_flash_send_halfword(uint16_t half_word);
/* enable the write access to the flash */
void spi_flash_write_enable(void);
/* poll the status of the write in progress (wip) flag in the flash's status register */
void spi_flash_wait_for_write_end(void);

/* 以下是使用 DMA 传输的函数 */
/* 使用 DMA 发送并接收一个字节 */
uint8_t spi_flash_send_byte_dma(uint8_t byte);
/* 使用 DMA 发送并接收一个半字 */
uint16_t spi_flash_send_halfword_dma(uint16_t half_word);
/* 使用 DMA 发送和接收多个字节 */
void spi_flash_transmit_receive_dma(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t size);
/* 等待 DMA 传输完成 */
void spi_flash_wait_for_dma_end(void);

#endif /* GD25QXX_H */
