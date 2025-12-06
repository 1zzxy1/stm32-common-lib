# SimpleUART通信库

> 📦 **Python工具库** - 完整的UART管理系统

## 功能特性

- 线程安全的串口管理
- 自动缓冲区刷新模式
- 帧格式支持（header/tail）
- 重试机制（最多3次）
- 数据自动提取解析（key:value）
- 变量绑定系统

## 使用场景

✅ 上位机串口通信、数据采集系统、测试工具
⚠️ 不适用STM32下位机（Python库）

## API示例

```python
from micu_uart_lib import SimpleUART

# 初始化串口
uart = SimpleUART('/dev/ttyUSB0', 115200)
uart.init()

# 发送数据（自动添加帧头帧尾）
uart.send("Hello STM32")

# 接收单帧
frame = uart.receive()
print(f"Received: {frame}")

# 接收所有帧
frames = uart.receive_all()

# 提取key:value数据
data = uart.extract_data_from_buffer()
print(f"Data: {data}")

# 变量绑定（自动更新）
from micu_uart_lib.utils import bind_variable

container = {}
bind_variable(container, 'temperature', 'float')
bind_variable(container, 'voltage', 'int')

# 关闭串口
uart.close()
```

## 配置

```python
from micu_uart_lib.config import Config

# 修改配置
Config.DEVICE = '/dev/ttyUSB0'
Config.BAUDRATE = 115200
Config.BUFFER_SIZE = 1024
Config.REFRESH_INTERVAL = 500  # ms

# 帧格式配置
Config.FRAME_HEADER = '$$'
Config.FRAME_TAIL = '##'
Config.FRAME_ENABLED = True
```

## 主要API

| 类/函数 | 功能 |
|---------|------|
| `SimpleUART(port, baud)` | 初始化串口 |
| `init()` | 打开串口 |
| `send(data)` | 发送数据 |
| `receive()` | 接收单帧 |
| `receive_all()` | 接收所有帧 |
| `extract_data_from_buffer()` | 提取key:value |
| `bind_variable(c, k, t)` | 绑定变量 |
| `micu_printf(msg)` | UART调试输出 |

## 特性

- **线程安全**：使用锁保护共享资源
- **自动重试**：发送失败自动重试3次
- **帧格式**：默认`$$data##`格式
- **数据解析**：自动解析`key:value`对

## 依赖项

- Python 3.6+
- pyserial
