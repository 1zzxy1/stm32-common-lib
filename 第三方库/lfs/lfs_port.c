/**
 * @file lfs_port.c
 * @author 米醋电子工作室 (mvk@mvktech.com)
 * @brief LittleFS porting layer for GD25QXX
 * @copyright Copyright (c) 2024 米醋电子工作室
 */
#include "lfs_port.h"
#include "gd25qxx.h" // 包含 GD25QXX 驱动头文件
#include <stdio.h>   // For NULL

// LittleFS 静态缓冲区 (避免 malloc)
// 缓冲区大小基于 lfs_port.h 中的定义
static uint8_t lfs_read_buffer[LFS_FLASH_PAGE_SIZE];
static uint8_t lfs_prog_buffer[LFS_FLASH_PAGE_SIZE];
static uint8_t lfs_lookahead_buffer[256 / 8]; // 假设 lookahead_size 为 256，lookahead_size 必须是8的倍数

// LittleFS block device operations
static int lfs_deskio_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    (void)c; // Unused parameter
    spi_flash_buffer_read(buffer, (block * LFS_FLASH_SECTOR_SIZE) + off, size);
    return LFS_ERR_OK;
}

static int lfs_deskio_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    (void)c; // Unused parameter
    spi_flash_buffer_write((uint8_t *)buffer, (block * LFS_FLASH_SECTOR_SIZE) + off, size);
    return LFS_ERR_OK;
}

static int lfs_deskio_erase(const struct lfs_config *c, lfs_block_t block)
{
    (void)c; // Unused parameter
    spi_flash_sector_erase(block * LFS_FLASH_SECTOR_SIZE);
    return LFS_ERR_OK;
}

static int lfs_deskio_sync(const struct lfs_config *c)
{
    (void)c; // Unused parameter
    // gd25qxx driver's write functions are blocking and wait for write completion.
    // If not, add a call to spi_flash_wait_for_write_end() or similar here.
    return LFS_ERR_OK;
}

// LittleFS configuration function
int lfs_storage_init(struct lfs_config *cfg)
{
    if (!cfg)
        return LFS_ERR_INVAL;

    cfg->context = NULL; // No specific context needed for this port

    // block device operations
    cfg->read = lfs_deskio_read;
    cfg->prog = lfs_deskio_prog;
    cfg->erase = lfs_deskio_erase;
    cfg->sync = lfs_deskio_sync;

    // block device configuration
    cfg->read_size = LFS_FLASH_PAGE_SIZE;
    cfg->prog_size = LFS_FLASH_PAGE_SIZE;
    cfg->block_size = LFS_FLASH_SECTOR_SIZE;
    cfg->block_count = LFS_BLOCK_COUNT;
    cfg->cache_size = LFS_FLASH_PAGE_SIZE;
    cfg->lookahead_size = sizeof(lfs_lookahead_buffer) * 8; // lookahead_size is in bits internally for buffer sizing, but config is in bytes
    cfg->block_cycles = 500;

    // Assign static buffers
    cfg->read_buffer = lfs_read_buffer;
    cfg->prog_buffer = lfs_prog_buffer;
    cfg->lookahead_buffer = lfs_lookahead_buffer;

    cfg->name_max = 0; // Use LFS_NAME_MAX default
    cfg->file_max = 0; // Use LFS_FILE_MAX default
    cfg->attr_max = 0; // Use LFS_ATTR_MAX default

    return LFS_ERR_OK;
}


