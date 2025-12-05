# GD25QXX SPI Flash驱动

> ⚠️ **硬件依赖模块** - 需要根据硬件配置修改GPIO和SPI

GD25Qxx系列SPI Flash芯片驱动，支持读写擦除操作。

## 特性

- 支持扇区擦除和批量擦除
- 页编程（256字节）
- 连续读取
- Flash ID读取
- 写保护控制

## 硬件依赖

**必须修改的宏定义**（在gd25qxx.h中）:
```c
#define SPI_FLASH_CS_LOW()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define SPI_FLASH_CS_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
```

**外部变量**（在gd25qxx.c中）:
```c
extern SPI_HandleTypeDef hspi2;  // 需要在main.c中定义
```

## 芯片型号

支持GD25Qxx系列（如GD25Q16、GD25Q32、GD25Q64、GD25Q128等）

## 使用方法

### 1. 配置硬件

在STM32CubeMX中：
- 配置SPI外设（如SPI2）
- 配置CS引脚为GPIO输出

### 2. 修改代码

修改`gd25qxx.h`中的CS控制宏和`gd25qxx.c`中的SPI句柄名。

### 3. 初始化

```c
#include "gd25qxx.h"

void Flash_Init(void)
{
    spi_flash_init();

    // 读取Flash ID验证通信
    uint32_t id = spi_flash_read_id();
    printf("Flash ID: 0x%06X\r\n", id);
}
```

### 4. 读写操作

```c
uint8_t write_buf[256] = "Hello Flash";
uint8_t read_buf[256];

// 擦除扇区（4KB）
spi_flash_sector_erase(0x000000);

// 写入数据
spi_flash_buffer_write(write_buf, 0x000000, sizeof(write_buf));

// 读取数据
spi_flash_buffer_read(read_buf, 0x000000, sizeof(read_buf));
```

## API概览

| 函数 | 说明 |
|------|------|
| `spi_flash_init()` | 初始化Flash |
| `spi_flash_read_id()` | 读取Flash ID |
| `spi_flash_sector_erase()` | 擦除扇区（4KB） |
| `spi_flash_bulk_erase()` | 擦除整个芯片 |
| `spi_flash_page_write()` | 页编程（≤256字节） |
| `spi_flash_buffer_write()` | 缓冲区写入（任意长度） |
| `spi_flash_buffer_read()` | 缓冲区读取 |

## 注意事项

1. 写入前必须先擦除（Flash特性：只能将1写为0）
2. 页编程不能跨页边界
3. 扇区大小通常为4KB
4. 写入操作后需等待完成（内部已处理）
5. CS引脚和SPI句柄必须根据实际硬件修改

## 常见Flash型号

| 型号 | 容量 | ID |
|------|------|-----|
| GD25Q16 | 2MB | 0xC84015 |
| GD25Q32 | 4MB | 0xC84016 |
| GD25Q64 | 8MB | 0xC84017 |
| GD25Q128 | 16MB | 0xC84018 |
