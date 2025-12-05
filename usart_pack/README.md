# USART Pack 串口数据包协议

通用的串口数据帧打包与解包模块，支持多种数据类型的灵活组合。

## 特性

- 支持多种数据类型：BYTE、SHORT、INT、FLOAT
- 模板化设计，运行时灵活配置
- 包含帧头、校验和、帧尾校验
- 大端模式传输整型数据
- 纯C实现，无硬件依赖
- 支持多协议实例

## 帧格式

```
[帧头 1B] [数据区 nB] [校验和 1B] [帧尾 1B]
  0xA5      ...          sum        0x5A
```

校验和 = 数据区所有字节的累加和 & 0xFF

## 使用方法

### 1. 定义协议和变量

```c
#include "usart_pack.h"

// 定义通信变量
uint8_t cmd;
uint16_t value1;
float value2;

// 定义协议实例
usart_pack_t protocol;
```

### 2. 初始化并配置模板

**方法A: 使用数组配置**
```c
void Protocol_Init(void)
{
    // 初始化协议
    usart_pack_init(&protocol);

    // 定义模板
    usart_pack_type_t types[] = {PACK_TYPE_BYTE, PACK_TYPE_SHORT, PACK_TYPE_FLOAT};
    void *vars[] = {&cmd, &value1, &value2};

    // 设置模板
    usart_pack_set_template(&protocol, types, vars, 3);
}
```

**方法B: 逐个添加变量**
```c
void Protocol_Init(void)
{
    usart_pack_init(&protocol);

    usart_pack_add_var(&protocol, PACK_TYPE_BYTE, &cmd);
    usart_pack_add_var(&protocol, PACK_TYPE_SHORT, &value1);
    usart_pack_add_var(&protocol, PACK_TYPE_FLOAT, &value2);
}
```

### 3. 发送数据

```c
uint8_t tx_buffer[64];

// 设置要发送的值
cmd = 0x01;
value1 = 1234;
value2 = 3.14f;

// 打包数据帧
uint16_t len = usart_pack_build(&protocol, tx_buffer, sizeof(tx_buffer));

// 通过串口发送
HAL_UART_Transmit(&huart1, tx_buffer, len, HAL_MAX_DELAY);
```

### 4. 接收解析

```c
void Process_Received_Data(uint8_t *rx_buf, uint16_t len)
{
    // 解析数据帧
    int result = usart_pack_parse(&protocol, rx_buf, len);

    if (result == 0) {
        // 解析成功，变量已自动更新
        printf("cmd=%d, value1=%d, value2=%.2f\n", cmd, value1, value2);
    } else if (result == -1) {
        printf("Frame format error\n");
    } else if (result == -2) {
        printf("Checksum error\n");
    }
}
```

### 5. 自定义帧头帧尾

```c
// 使用自定义帧头0xAA和帧尾0x55
usart_pack_init_custom(&protocol, 0xAA, 0x55);
```

### 6. 多协议实例

```c
usart_pack_t cmd_protocol;   // 命令协议
usart_pack_t data_protocol;  // 数据协议

void Multi_Protocol_Init(void)
{
    // 命令协议：单字节命令
    usart_pack_init(&cmd_protocol);
    usart_pack_add_var(&cmd_protocol, PACK_TYPE_BYTE, &cmd);

    // 数据协议：多数据
    usart_pack_init(&data_protocol);
    usart_pack_add_var(&data_protocol, PACK_TYPE_FLOAT, &sensor1);
    usart_pack_add_var(&data_protocol, PACK_TYPE_FLOAT, &sensor2);
}
```

## 配置选项

在包含头文件前定义以修改默认配置：

```c
#define USART_PACK_MAX_VARIABLES 32  // 最大变量数
#define USART_PACK_HEADER 0xAA       // 帧头
#define USART_PACK_TAIL 0x55         // 帧尾
#include "usart_pack.h"
```

## 数据类型

| 类型 | 枚举值 | 字节数 | 说明 |
|------|--------|--------|------|
| BYTE | `PACK_TYPE_BYTE` | 1 | uint8_t / int8_t |
| SHORT | `PACK_TYPE_SHORT` | 2 | uint16_t / int16_t (大端) |
| INT | `PACK_TYPE_INT` | 4 | uint32_t / int32_t (大端) |
| FLOAT | `PACK_TYPE_FLOAT` | 4 | float (IEEE 754) |

## API 概览

| 函数 | 说明 |
|------|------|
| `usart_pack_init()` | 初始化协议实例 |
| `usart_pack_init_custom()` | 自定义帧头帧尾初始化 |
| `usart_pack_set_template()` | 设置完整模板 |
| `usart_pack_add_var()` | 添加单个变量 |
| `usart_pack_clear()` | 清空模板 |
| `usart_pack_parse()` | 解析数据帧 |
| `usart_pack_build()` | 打包数据帧 |
| `usart_pack_calc_data_size()` | 计算数据区大小 |
| `usart_pack_calc_frame_size()` | 计算完整帧大小 |

## 注意事项

1. SHORT和INT类型使用大端模式传输，与上位机通信时注意字节序
2. FLOAT类型使用小端模式（IEEE 754），与MCU内存布局一致
3. 解析成功后，变量会被自动更新，无需手动拷贝
4. 发送前需确保变量已赋值
