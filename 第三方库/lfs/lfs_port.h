/**
 * @file lfs_port.h
 * @author 米醋电子工作室 (mvk@mvktech.com)
 * @brief LittleFS port for GD25QXX
 * @copyright Copyright (c) 2024 米醋电子工作室
 */
#ifndef LFS_PORT_H
#define LFS_PORT_H

#include "lfs.h"

// 定义 Flash 相关参数，请根据实际的 Flash 型号修改
#define LFS_FLASH_TOTAL_SIZE (2 * 1024 * 1024) // 例如 GD25Q16 (16Mbit = 2MB)
#define LFS_FLASH_SECTOR_SIZE (4 * 1024)       // 4KB sector size
#define LFS_FLASH_PAGE_SIZE (256)              // 256B page size (prog/read size)

#define LFS_BLOCK_COUNT (LFS_FLASH_TOTAL_SIZE / LFS_FLASH_SECTOR_SIZE)

// LittleFS 配置函数
int lfs_storage_init(struct lfs_config *cfg);

#endif // LFS_PORT_H