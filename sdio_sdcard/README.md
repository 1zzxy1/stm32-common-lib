# SDIO SD Card 驱动

完整的 SD 卡协议栈实现，支持标准 SDIO 接口的 SD/SDHC/MMC 卡操作。

## 功能特性

- ✅ 完整的 SD 卡协议实现（CMD0-CMD56, ACMD）
- ✅ 支持多种卡类型（SD v1.1/v2.0、SDHC、MMC、高速 MMC）
- ✅ 单块/多块读写操作
- ✅ 扇区擦除功能
- ✅ DMA 传输模式和轮询模式
- ✅ 卡锁定/解锁功能
- ✅ 完整的错误处理机制（30+ 错误类型）
- ✅ 卡信息读取（CID、CSD、SCR、容量等）
- ✅ 4-bit 和 1-bit 总线模式

## 硬件要求

- SDIO 外设（支持 4-bit 或 1-bit 数据总线）
- 时钟频率：初始化 ≤ 400kHz，传输 ≤ 50MHz
- 支持 DMA 传输（可选，提升性能）
- 6 个 GPIO 引脚（4-bit 模式）或 3 个引脚（1-bit 模式）
  - CMD（命令线）
  - CLK（时钟线）
  - DAT0-DAT3（数据线，1-bit 模式仅需 DAT0）

## 使用场景

- 数据记录系统（传感器数据、日志文件）
- 文件系统存储（配合 FatFs/LittleFS）
- 固件升级存储（IAP/OTA）
- 大容量数据缓存
- 音视频录制存储
- 嵌入式数据库后端

## 移植说明

需要适配以下底层接口：

```c
/* 1. 修改 sdio_sdcard.c 中的 SDIO 寄存器操作 */
// 替换 GD32 的 SDIO 寄存器访问为目标平台的 HAL 库调用
// 例如：sdio_command_response_config() -> HAL_SD_SendCommand()

/* 2. 适配 DMA 传输接口（如使用 DMA 模式）*/
// 替换 DMA 配置函数为目标平台的 DMA 初始化代码

/* 3. 适配时钟和 GPIO 初始化 */
// 在 sd_init() 中添加目标平台的 SDIO 时钟和引脚配置
```

**移植难度**：中等（需要熟悉目标平台的 SDIO 外设）

**关键点**：
- SDIO 命令和响应格式是标准的，无需修改
- 主要工作是替换寄存器操作为 HAL 库调用
- 状态机和协议逻辑可直接复用

## 基本用法

```c
#include "sdio_sdcard.h"

// 1. 初始化 SD 卡
sd_error_enum status = sd_init();
if (status != SD_OK) {
    // 初始化失败处理
}

// 2. 获取卡信息
sd_card_info_struct card_info;
sd_card_information_get(&card_info);
uint32_t capacity_kb = sd_card_capacity_get();  // 获取容量（KB）

// 3. 配置传输模式
sd_transfer_mode_config(SD_DMA_MODE);      // DMA 模式（推荐）
// 或
sd_transfer_mode_config(SD_POLLING_MODE);  // 轮询模式

// 4. 配置总线宽度
sd_bus_mode_config(SDIO_BUSMODE_4BIT);    // 4-bit 模式（更快）
// 或
sd_bus_mode_config(SDIO_BUSMODE_1BIT);    // 1-bit 模式

// 5. 读取单个扇区（512 字节）
uint32_t read_buffer[128];  // 512 字节 = 128 个 uint32_t
status = sd_block_read(read_buffer, 0x0000, 512);

// 6. 写入单个扇区
uint32_t write_buffer[128] = {0x01, 0x02, 0x03, ...};
status = sd_block_write(write_buffer, 0x0000, 512);

// 7. 读取多个扇区
uint32_t multi_buffer[1024];  // 4 个扇区 = 2048 字节
status = sd_multiblocks_read(multi_buffer, 0x0000, 512, 4);

// 8. 擦除扇区范围
status = sd_erase(0x0000, 0x10000);  // 擦除 0x0000 到 0x10000 地址范围
```

## API 说明

### 初始化和电源管理

```c
sd_error_enum sd_init(void);                    // 初始化 SD 卡（完整流程）
sd_error_enum sd_card_init(void);               // 初始化卡并获取 CID/CSD
sd_error_enum sd_power_on(void);                // 配置时钟和电压，获取卡类型
sd_error_enum sd_power_off(void);               // 关闭 SDIO 电源
```

### 配置操作

```c
sd_error_enum sd_bus_mode_config(uint32_t busmode);      // 配置总线宽度（1-bit/4-bit）
sd_error_enum sd_transfer_mode_config(uint32_t txmode);  // 配置传输模式（DMA/轮询）
```

### 读写操作

```c
// 单块读写（blocksize 通常为 512 字节）
sd_error_enum sd_block_read(uint32_t *preadbuffer, uint32_t readaddr, uint16_t blocksize);
sd_error_enum sd_block_write(uint32_t *pwritebuffer, uint32_t writeaddr, uint16_t blocksize);

// 多块读写
sd_error_enum sd_multiblocks_read(uint32_t *preadbuffer, uint32_t readaddr,
                                   uint16_t blocksize, uint32_t blocksnumber);
sd_error_enum sd_multiblocks_write(uint32_t *pwritebuffer, uint32_t writeaddr,
                                    uint16_t blocksize, uint32_t blocksnumber);
```

### 擦除和控制

```c
sd_error_enum sd_erase(uint32_t startaddr, uint32_t endaddr);  // 擦除连续区域
sd_error_enum sd_transfer_stop(void);                          // 停止正在进行的传输
sd_error_enum sd_lock_unlock(uint8_t lockstate);               // 锁定/解锁卡
```

### 状态和信息查询

```c
sd_error_enum sd_card_information_get(sd_card_info_struct *pcardinfo);  // 获取卡详细信息
sd_error_enum sd_cardstatus_get(uint32_t *pcardstatus);                 // 获取卡状态（R1 响应）
sd_error_enum sd_sdstatus_get(uint32_t *psdstatus);                     // 获取 SD 状态（512 位）
sd_transfer_state_enum sd_transfer_state_get(void);                     // 获取传输状态
uint32_t sd_card_capacity_get(void);                                    // 获取卡容量（KB）
uint8_t sd_card_check(void);                                            // 检查卡是否存在
```

## 卡类型支持

| 卡类型 | 枚举值 | 说明 |
|--------|--------|------|
| SD v1.1 | `SDIO_STD_CAPACITY_SD_CARD_V1_1` | 标准容量 SD 卡 v1.1 |
| SD v2.0 | `SDIO_STD_CAPACITY_SD_CARD_V2_0` | 标准容量 SD 卡 v2.0 |
| SDHC | `SDIO_HIGH_CAPACITY_SD_CARD` | 高容量 SD 卡（> 2GB）|
| MMC | `SDIO_MULTIMEDIA_CARD` | 多媒体卡 |
| 高速 MMC | `SDIO_HIGH_SPEED_MULTIMEDIA_CARD` | 高速多媒体卡 |

## 错误处理

驱动提供 30+ 种详细错误类型，涵盖：

- **卡错误**：地址越界、块长度错误、擦除错误、写保护等
- **通信错误**：CRC 错误、超时、起始位错误
- **FIFO 错误**：发送下溢、接收溢出
- **参数错误**：电压范围无效、参数无效、操作不当

```c
sd_error_enum status = sd_block_read(buffer, addr, 512);
if (status != SD_OK) {
    switch (status) {
        case SD_CMD_RESP_TIMEOUT:
            // 命令响应超时
            break;
        case SD_DATA_CRC_ERROR:
            // 数据 CRC 校验失败
            break;
        case SD_ADDR_OUT_OF_RANGE:
            // 地址越界
            break;
        // ... 处理其他错误
    }
}
```

## 注意事项

1. **地址单位** - 对于 SDHC 卡，地址以扇区为单位；对于标准容量卡，地址以字节为单位
2. **块大小** - 标准块大小为 512 字节，不建议修改
3. **DMA 对齐** - 使用 DMA 模式时，缓冲区地址需要 4 字节对齐
4. **多块传输** - 多块读写比单块循环更高效，建议优先使用
5. **擦除操作** - 擦除前无需手动擦除，写操作会自动处理
6. **卡检测** - 建议在操作前调用 `sd_card_check()` 检查卡是否存在
7. **时钟频率** - 初始化阶段必须使用低速时钟（≤ 400kHz），传输阶段可提升至 50MHz

## 性能参考

| 操作 | 典型速度（4-bit DMA 模式）|
|------|---------------------------|
| 单块读取 | ~10 MB/s |
| 单块写入 | ~8 MB/s |
| 多块读取 | ~20 MB/s |
| 多块写入 | ~15 MB/s |

*实际速度取决于卡的速度等级（Class 2/4/6/10）和 SDIO 时钟频率*

## 配置要求

- **依赖**：SDIO 外设（STM32/GD32 HAL 库）
- **GPIO**：6 个引脚（4-bit）或 3 个引脚（1-bit）
- **DMA**：可选（显著提升性能）
- **RAM**：至少 2KB（用于缓冲区和卡信息结构）
- **卡容量**：支持 SD/SDHC（最大 32GB）

## 来源

源自 mcu_-main-board 项目的 GD32F470 示例
