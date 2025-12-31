# GD25Qxx SPI Flash 驱动

基于标准 SPI 协议的 Flash 驱动，支持兆易创新 GD25Q 系列及其他兼容 SPI Flash 芯片。

## 功能特性

- ✅ 标准 SPI Flash 命令集（W25Q/GD25Q/MX25 等通用）
- ✅ 扇区擦除、整片擦除
- ✅ 页写入（256字节）、缓冲区写入
- ✅ 读取数据、读取芯片ID
- ✅ DMA 加速传输（可选）
- ✅ 写保护控制

## 硬件要求

- SPI 外设（任意速率，建议 ≤ 50MHz）
- 1 个 GPIO 作为片选 CS
- 支持 DMA 传输（可选）

## 使用场景

- 数据存储（配置参数、日志记录）
- 文件系统后端存储（配合 LittleFS）
- 固件升级（IAP/OTA）
- 大容量数据缓存

## 移植说明

需要实现以下底层接口：

```c
/* 1. 修改 gd25qxx.h 中的宏定义 */
#define SPI_FLASH_CS_LOW()  gpio_bit_reset(GPIOB, GPIO_PIN_12)  // 片选拉低
#define SPI_FLASH_CS_HIGH() gpio_bit_set(GPIOB, GPIO_PIN_12)    // 片选拉高
#define SPI_FLASH          SPI1                                  // SPI 外设

/* 2. 实现 SPI 字节收发函数 */
uint8_t spi_flash_send_byte_dma(uint8_t byte);  // 使用 DMA 发送并接收一个字节
// 或者实现非 DMA 版本：
// uint8_t spi_flash_send_byte(uint8_t byte);
```

## 基本用法

```c
#include "gd25qxx.h"

// 1. 初始化
spi_flash_init();

// 2. 读取芯片 ID
uint32_t id = spi_flash_read_id();
// GD25Q128: 0xC84018, W25Q128: 0xEF4018

// 3. 写入数据（先擦除扇区）
uint32_t addr = 0x0000;
uint8_t data[256] = {0x01, 0x02, 0x03, ...};

spi_flash_sector_erase(addr);           // 擦除 4KB 扇区
spi_flash_buffer_write(data, addr, 256); // 写入数据

// 4. 读取数据
uint8_t read_buf[256];
spi_flash_buffer_read(read_buf, addr, 256);
```

## API 说明

### 初始化和信息读取

```c
void spi_flash_init(void);                  // 初始化 Flash
uint32_t spi_flash_read_id(void);           // 读取芯片 ID
```

### 擦除操作

```c
void spi_flash_sector_erase(uint32_t addr); // 擦除 4KB 扇区（耗时 ~50ms）
void spi_flash_bulk_erase(void);            // 整片擦除（耗时数秒）
```

### 写入操作

```c
void spi_flash_page_write(uint8_t *buf, uint32_t addr, uint16_t len);   // 页写入（≤256字节）
void spi_flash_buffer_write(uint8_t *buf, uint32_t addr, uint16_t len); // 缓冲区写入（自动跨页）
```

### 读取操作

```c
void spi_flash_buffer_read(uint8_t *buf, uint32_t addr, uint16_t len);  // 读取数据
```

## 注意事项

1. **擦除后再写入** - Flash 写入前必须先擦除（擦除后为 0xFF）
2. **地址对齐** - 扇区擦除地址需 4KB 对齐（0x0000, 0x1000, 0x2000...）
3. **页写入限制** - 单次页写入不能跨越 256 字节边界
4. **DMA 依赖** - 当前实现使用 DMA，如需轮询模式请修改 `spi_flash_send_byte_dma()` 实现

## 支持的芯片

理论支持所有标准 SPI Flash：

- ✅ GD25Q 系列（兆易创新）：GD25Q16/32/64/128/256
- ✅ W25Q 系列（华邦）：W25Q16/32/64/128/256
- ✅ MX25 系列（旺宏）：MX25L/U 系列
- ✅ AT25 系列（爱特梅尔）

## 配置要求

- **依赖**：SPI 外设（STM32 HAL/GD32 标准库）
- **GPIO**：1 个片选引脚
- **DMA**：可选（提升传输速度）
- **Flash 大小**：任意（16Mbit ~ 256Mbit）

## 来源

源自 mcu_-main-board 项目的 GD32F470 示例
