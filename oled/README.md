# OLED 基础驱动

> 🖥️ **硬件依赖模块** - SSD1306 OLED显示屏驱动

0.96寸 OLED显示屏（SSD1306芯片）的基础驱动，支持I2C/SPI接口。

## 特性

- 分辨率：128×64
- 支持I2C和SPI接口
- 基础图形函数（点、线、矩形、圆）
- 字符显示（ASCII 6×8、8×16字体）
- 中文显示（16×16点阵）
- 图片显示（BMP）
- 屏幕翻转、对比度调节

## 硬件依赖

**需要根据实际接口修改驱动**:

### I2C接口（常用）
```c
// 修改oled.c中的I2C相关函数
extern I2C_HandleTypeDef hi2c1;  // I2C句柄

#define OLED_I2C_ADDRESS 0x78    // OLED I2C地址
```

### SPI接口
```c
// 修改oled.c中的SPI和GPIO控制
extern SPI_HandleTypeDef hspi1;  // SPI句柄

#define OLED_DC_PIN   GPIO_PIN_x
#define OLED_DC_PORT  GPIOx
#define OLED_RES_PIN  GPIO_PIN_x
#define OLED_RES_PORT GPIOx
```

## 文件说明

```
oled/
├── oled.c         # OLED驱动实现
├── oled.h         # OLED驱动头文件
├── oledfont.h     # ASCII字体数据
├── oledpic.h      # 图片数据（可选）
└── README.md
```

## 使用方法

### 1. 初始化

```c
#include "oled.h"

void OLED_Init_App(void)
{
    OLED_Init();       // 初始化OLED
    OLED_Clear();      // 清屏
}
```

### 2. 显示字符

```c
// 显示单个字符 (6×8)
OLED_ShowChar(0, 0, 'A');

// 显示字符串
OLED_ShowString(0, 0, "Hello OLED");

// 显示数字
OLED_ShowNum(0, 2, 12345, 5);
```

### 3. 显示中文

```c
// 显示16×16中文（需要提前在字库中定义）
OLED_ShowCHinese(0, 0, 0);  // 显示第0个中文字符
```

### 4. 绘制图形

```c
// 画点
OLED_DrawPoint(10, 10);

// 画线
OLED_DrawLine(0, 0, 127, 63);

// 画矩形
OLED_DrawRect(10, 10, 50, 30);

// 画圆
OLED_DrawCircle(64, 32, 20);

// 填充矩形
OLED_Fill(0, 0, 127, 7, 0xFF);  // 填充第一页
```

### 5. 显示图片

```c
// 显示预存的BMP图片
OLED_ShowPicture(0, 0, 128, 8, BMP1);
```

## API概览

### 初始化和控制
- `OLED_Init()` - 初始化OLED
- `OLED_Clear()` - 清屏
- `OLED_Display_On()` - 开显示
- `OLED_Display_Off()` - 关显示
- `OLED_Set_Pos()` - 设置坐标
- `OLED_Refresh_Gram()` - 刷新显存

### 字符显示
- `OLED_ShowChar()` - 显示单个字符
- `OLED_ShowString()` - 显示字符串
- `OLED_ShowNum()` - 显示数字
- `OLED_ShowCHinese()` - 显示中文

### 图形绘制
- `OLED_DrawPoint()` - 画点
- `OLED_DrawLine()` - 画线
- `OLED_DrawRect()` - 画矩形
- `OLED_DrawCircle()` - 画圆
- `OLED_Fill()` - 区域填充
- `OLED_ShowPicture()` - 显示图片

## 显存结构

OLED显示采用页寻址模式：
- 分辨率：128列 × 64行
- 分页：8页（每页8行）
- 显存：128字节/页 × 8页 = 1KB

```
页0: 行0-7
页1: 行8-15
页2: 行16-23
...
页7: 行56-63
```

## 字体说明

### ASCII字体（oledfont.h）
- 6×8点阵：适合小字显示
- 8×16点阵：标准显示

### 中文字体
需要自行添加汉字点阵数据：

```c
// 在oledfont.h中添加
const unsigned char Hzk[][32]={
    {0x00,0x00,...},  // "中"字的16×16点阵
    {0x00,0x00,...},  // "文"字的16×16点阵
};
```

## 典型应用

### 信息显示
```c
void Display_Info(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "Voltage:");
    OLED_ShowString(70, 0, "3.30V");
    OLED_ShowString(0, 2, "Current:");
    OLED_ShowString(70, 2, "0.5A");
    OLED_ShowString(0, 4, "Power:");
    OLED_ShowString(70, 4, "1.65W");
}
```

### 动画显示
```c
void Animate_Circle(void)
{
    for (int r = 1; r <= 30; r++) {
        OLED_Clear();
        OLED_DrawCircle(64, 32, r);
        OLED_Refresh_Gram();
        HAL_Delay(50);
    }
}
```

### 波形显示
```c
void Display_Waveform(uint8_t *data, uint16_t len)
{
    OLED_Clear();
    for (int i = 0; i < len - 1 && i < 127; i++) {
        OLED_DrawLine(i, 63 - data[i]/4,
                      i+1, 63 - data[i+1]/4);
    }
    OLED_Refresh_Gram();
}
```

## 接口切换

### I2C模式（推荐）
- 优点：只需2根线（SDA、SCL）
- 缺点：速度较慢
- 适用：信息显示

### SPI模式
- 优点：速度快
- 缺点：需要更多引脚
- 适用：动画、图形

## 配置步骤

### STM32CubeMX配置

**I2C模式**:
1. 启用I2C1
2. 速度：400kHz（Fast Mode）
3. 地址：7-bit

**SPI模式**:
1. 启用SPI1
2. 模式：Master
3. 速度：≤10MHz
4. 配置DC和RES引脚为GPIO输出

## 注意事项

1. I2C地址可能是0x78或0x3C（取决于硬件）
2. 初始化时需要延时等待OLED上电
3. 中文显示需要自行添加字库
4. 图片数据占用较大Flash空间
5. 刷新频率过高可能闪烁
6. SPI模式需要正确控制DC引脚

## 性能指标

- 刷新率：I2C模式约30fps，SPI模式约60fps
- 显示延迟：<50ms
- 对比度：256级可调
- 功耗：<20mA（全亮）

## 内存占用

- 代码：~5KB
- RAM：1KB（显存缓冲区）+ 字库数据（Flash）
- Stack：<100字节

## 扩展功能

### 添加自定义字体
1. 使用字模软件生成点阵数据
2. 添加到oledfont.h
3. 调用OLED_ShowCHinese()显示

### 添加图片
1. 使用Image2Lcd工具转换BMP
2. 格式：128×64，单色
3. 添加到oledpic.h

## 常见问题

**Q: OLED不显示？**
A: 检查I2C地址、接线、电源

**Q: 显示乱码？**
A: 检查字符编码、字库数据

**Q: 刷新慢？**
A: 使用SPI模式或降低刷新频率

**Q: 中文显示不正常？**
A: 确认字库数据正确、字号匹配
