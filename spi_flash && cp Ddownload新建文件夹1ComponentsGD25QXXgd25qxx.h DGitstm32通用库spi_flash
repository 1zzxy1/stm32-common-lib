#include "gd25qxx.h"
#include "stm32f4xx_hal.h"
#include <string.h>

#define WRITE 0x02 /* write to memory instruction */
#define WRSR 0x01  /* write status register instruction */
#define WREN 0x06  /* write enable instruction */

#define READ 0x03 /* read from memory instruction */
#define RDSR 0x05 /* read status register instruction  */
#define RDID 0x90 /* read identification */
#define SE 0x20   /* sector erase instruction */
#define BE 0xC7   /* bulk erase instruction */

#define WIP_FLAG 0x01 /* write in progress(wip)flag */
#define DUMMY_BYTE 0xA5

/* SPI handle */
extern SPI_HandleTypeDef hspi2;

/**
 * @brief Initializes the SPI Flash chip.
 * @note This function assumes that the SPI peripheral (e.g., hspi2) and CS GPIO
 *       have already been initialized by the STM32CubeMX generated code (e.g., MX_SPI2_Init(), MX_GPIO_Init()).
 *       It primarily ensures the CS pin is high (chip deselected) initially.
 *       You can add a Flash ID read here for an initial check if desired.
 */
void spi_flash_init(void)
{
    // Ensure CS pin is high (deselected) initially.
    // The GPIO for CS (PB12 in your macros) should be configured as output push-pull in MX_GPIO_Init.
    SPI_FLASH_CS_HIGH();
    // Optional: Add a small delay if needed after power-up or SPI init, before first command
    // HAL_Delay(1);

    // Optional: You could read the Flash ID here to verify communication
    // uint32_t id = spi_flash_read_id();
    // (Add code to check ID or print it for debugging)
}

void spi_flash_sector_erase(uint32_t sector_addr)
{
    spi_flash_write_enable();

    SPI_FLASH_CS_LOW();
    spi_flash_send_byte(SE);
    spi_flash_send_byte((sector_addr & 0xFF0000) >> 16);
    spi_flash_send_byte((sector_addr & 0xFF00) >> 8);
    spi_flash_send_byte(sector_addr & 0xFF);
    SPI_FLASH_CS_HIGH();

    spi_flash_wait_for_write_end();
}

void spi_flash_bulk_erase(void)
{
    spi_flash_write_enable();

    SPI_FLASH_CS_LOW();
    spi_flash_send_byte(BE);
    SPI_FLASH_CS_HIGH();

    spi_flash_wait_for_write_end();
}

void spi_flash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint16_t num_byte_to_write)
{
    spi_flash_write_enable();

    SPI_FLASH_CS_LOW();
    spi_flash_send_byte(WRITE);
    spi_flash_send_byte((write_addr & 0xFF0000) >> 16);
    spi_flash_send_byte((write_addr & 0xFF00) >> 8);
    spi_flash_send_byte(write_addr & 0xFF);

    while (num_byte_to_write--)
    {
        spi_flash_send_byte(*pbuffer);
        pbuffer++;
    }

    SPI_FLASH_CS_HIGH();
    spi_flash_wait_for_write_end();
}

void spi_flash_buffer_write(uint8_t *pbuffer, uint32_t write_addr, uint16_t num_byte_to_write)
{
    uint8_t num_of_page = 0, num_of_single = 0, addr = 0, count = 0, temp = 0;

    addr = write_addr % SPI_FLASH_PAGE_SIZE;
    count = SPI_FLASH_PAGE_SIZE - addr;
    num_of_page = num_byte_to_write / SPI_FLASH_PAGE_SIZE;
    num_of_single = num_byte_to_write % SPI_FLASH_PAGE_SIZE;

    if (0 == addr)
    {
        if (0 == num_of_page)
        {
            spi_flash_page_write(pbuffer, write_addr, num_byte_to_write);
        }
        else
        {
            while (num_of_page--)
            {
                spi_flash_page_write(pbuffer, write_addr, SPI_FLASH_PAGE_SIZE);
                write_addr += SPI_FLASH_PAGE_SIZE;
                pbuffer += SPI_FLASH_PAGE_SIZE;
            }
            spi_flash_page_write(pbuffer, write_addr, num_of_single);
        }
    }
    else
    {
        if (0 == num_of_page)
        {
            if (num_of_single > count)
            {
                temp = num_of_single - count;
                spi_flash_page_write(pbuffer, write_addr, count);
                write_addr += count;
                pbuffer += count;
                spi_flash_page_write(pbuffer, write_addr, temp);
            }
            else
            {
                spi_flash_page_write(pbuffer, write_addr, num_byte_to_write);
            }
        }
        else
        {
            num_byte_to_write -= count;
            num_of_page = num_byte_to_write / SPI_FLASH_PAGE_SIZE;
            num_of_single = num_byte_to_write % SPI_FLASH_PAGE_SIZE;

            spi_flash_page_write(pbuffer, write_addr, count);
            write_addr += count;
            pbuffer += count;

            while (num_of_page--)
            {
                spi_flash_page_write(pbuffer, write_addr, SPI_FLASH_PAGE_SIZE);
                write_addr += SPI_FLASH_PAGE_SIZE;
                pbuffer += SPI_FLASH_PAGE_SIZE;
            }

            if (0 != num_of_single)
            {
                spi_flash_page_write(pbuffer, write_addr, num_of_single);
            }
        }
    }
}

void spi_flash_buffer_read(uint8_t *pbuffer, uint32_t read_addr, uint16_t num_byte_to_read)
{
    SPI_FLASH_CS_LOW();
    spi_flash_send_byte(READ);
    spi_flash_send_byte((read_addr & 0xFF0000) >> 16);
    spi_flash_send_byte((read_addr & 0xFF00) >> 8);
    spi_flash_send_byte(read_addr & 0xFF);

    while (num_byte_to_read--)
    {
        *pbuffer = spi_flash_send_byte(DUMMY_BYTE);
        pbuffer++;
    }

    SPI_FLASH_CS_HIGH();
}

uint32_t spi_flash_read_id(void)
{
    uint32_t temp = 0, temp0 = 0, temp1 = 0, temp2 = 0;

    SPI_FLASH_CS_LOW();
    spi_flash_send_byte(RDID);
    temp0 = spi_flash_send_byte(DUMMY_BYTE);
    temp1 = spi_flash_send_byte(DUMMY_BYTE);
    temp2 = spi_flash_send_byte(DUMMY_BYTE);
    SPI_FLASH_CS_HIGH();

    temp = (temp0 << 16) | (temp1 << 8) | temp2;
    return temp;
}

void spi_flash_start_read_sequence(uint32_t read_addr)
{
    SPI_FLASH_CS_LOW();
    spi_flash_send_byte(READ);
    spi_flash_send_byte((read_addr & 0xFF0000) >> 16);
    spi_flash_send_byte((read_addr & 0xFF00) >> 8);
    spi_flash_send_byte(read_addr & 0xFF);
}

uint8_t spi_flash_read_byte(void)
{
    return (spi_flash_send_byte(DUMMY_BYTE));
}

uint8_t spi_flash_send_byte(uint8_t byte)
{
    uint8_t rx_data;
    HAL_SPI_TransmitReceive(&hspi2, &byte, &rx_data, 1, 1000);
    return rx_data;
}

uint16_t spi_flash_send_halfword(uint16_t half_word)
{
    uint16_t rx_data;
    HAL_SPI_TransmitReceive(&hspi2, (uint8_t *)&half_word, (uint8_t *)&rx_data, 2, 1000);
    return rx_data;
}

void spi_flash_write_enable(void)
{
    SPI_FLASH_CS_LOW();
    spi_flash_send_byte(WREN);
    SPI_FLASH_CS_HIGH();
}

void spi_flash_wait_for_write_end(void)
{
    uint8_t flash_status = 0;

    SPI_FLASH_CS_LOW();
    spi_flash_send_byte(RDSR);

    do
    {
        flash_status = spi_flash_send_byte(DUMMY_BYTE);
    } while ((flash_status & WIP_FLAG) == 0x01);

    SPI_FLASH_CS_HIGH();
}


