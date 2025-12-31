# LQ_OLED096 OLED显示驱动

> ⚙️ **硬件依赖模块** - 适用于STC16单片机平台

## 功能特性

- 0.96寸OLED显示屏驱动（SSD1306芯片）
- I2C/SPI双接口支持
- 128x64分辨率
- 丰富的显示函数库
- 内置中文字库和英文字体
- 图形绘制功能

## 使用场景

✅ **适用于**：
- STC16F系列单片机项目
- 龙邱智能车开发板
- 调试信息显示
- 参数实时监控
- 简单UI界面

⚠️ **不适用于**：
- STM32平台（需要修改I2C/SPI接口）
- 彩色屏幕显示
- 复杂图形界面（推荐使用u8g2+WouoUI）

## 配置说明

### 平台要求
- 单片机：STC16F系列
- 显示屏：0.96寸OLED SSD1306
- 通信接口：I2C或SPI（可选）
- 分辨率：128x64

### 必须修改的代码

需要根据硬件连接修改IO定义：

```c
// 在 LQ_OLED096.c 中修改
// I2C模式
#define OLED_SCL    P1^0  // I2C时钟
#define OLED_SDA    P1^1  // I2C数据

// SPI模式
#define OLED_CS     P1^0  // 片选
#define OLED_DC     P1^1  // 数据/命令选择
#define OLED_RST    P1^2  // 复位
```

## API使用示例

### 基本显示

```c
#include "LQ_OLED096.h"

void main(void)
{
    // 初始化OLED
    OLED_Init();

    // 清屏
    OLED_Clear();

    // 显示字符串
    OLED_ShowString(0, 0, "Hello World!", 16);

    // 显示数字
    OLED_ShowNum(0, 16, 12345, 5, 16);

    // 显示中文
    OLED_ShowChinese(0, 32, 0);  // 显示第0个汉字

    // 刷新显示
    OLED_Refresh();
}
```

### 图形绘制

```c
#include "LQ_OLED096.h"

void main(void)
{
    OLED_Init();

    // 画点
    OLED_DrawPoint(10, 10);

    // 画线
    OLED_DrawLine(0, 0, 127, 63);

    // 画矩形
    OLED_DrawRectangle(20, 20, 60, 40);

    // 画圆
    OLED_DrawCircle(64, 32, 20);

    OLED_Refresh();
}
```

### 实时数据显示

```c
void display_sensor_data(void)
{
    float angle = get_angle();
    int speed = get_speed();

    OLED_Clear();
    OLED_ShowString(0, 0, "Angle:", 16);
    OLED_ShowNum(60, 0, (int)angle, 3, 16);

    OLED_ShowString(0, 16, "Speed:", 16);
    OLED_ShowNum(60, 16, speed, 4, 16);

    OLED_Refresh();
}
```

## 性能参数

- 分辨率：128x64像素
- 显示颜色：单色（白光/蓝光/黄蓝双色）
- 刷新率：约100Hz
- 通信速率：I2C 400kHz / SPI 8MHz
- 字体大小：6x8 / 8x16 / 16x16（中文）
- 显存占用：1KB（128x64/8）

## 注意事项

1. **平台限制**：专为STC16平台设计，移植到其他平台需要修改底层接口
2. **显存刷新**：修改显示内容后需要调用OLED_Refresh()才能显示
3. **I2C地址**：默认0x78，如果显示不正常可尝试0x7A
4. **电源要求**：OLED需要3.3V供电，如果是5V单片机需要电平转换
5. **字库裁剪**：如果Flash空间紧张，可以删减不用的字体和汉字

## 依赖项

- STC16F头文件
- 龙邱STC16开发板底层驱动（LQ_I2C或LQ_SPI）

## 来源

网友那拿的（龙邱智能车例程）
