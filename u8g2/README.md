# U8g2 图形库

> 📦 **第三方开源库** - 功能强大的单色显示库

U8g2是一个适用于嵌入式设备的单色图形库，支持多种显示器。

## 特性

- 支持超过200种显示器型号
- 丰富的字体库（内置60+字体）
- Unicode支持
- 多种绘图函数
- 低RAM占用（页缓冲模式）
- 完整的图形用户界面（MUI）
- 高度可移植

## 支持的显示器

- SSD1306 (OLED 128×64)
- SSD1309
- SH1106
- ST7920 (LCD 128×64)
- UC1701
- PCD8544 (Nokia 5110)
- 等200+型号

## 文件说明

```
u8g2/
├── u8g2.h              # 主头文件
├── u8g2_*.c            # 核心功能实现
├── u8g2_fonts.c        # 字体数据（37MB！）
├── u8g2_d_*.c          # 设备驱动
├── u8x8_*.c            # 底层8×8字符显示
├── mui.c/mui.h         # 菜单界面库
├── u8log.c/u8log.h     # 日志功能
└── README.md
```

## 移植要求

**必须实现的回调函数**:

```c
// GPIO和延时回调
uint8_t u8x8_gpio_and_delay_template(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            // 初始化GPIO
            break;
        case U8X8_MSG_DELAY_MILLI:
            HAL_Delay(arg_int);
            break;
        case U8X8_MSG_GPIO_CS:
            // 控制CS引脚
            break;
        case U8X8_MSG_GPIO_DC:
            // 控制DC引脚
            break;
        // ... 其他消息
    }
    return 1;
}

// I2C/SPI通信回调
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg) {
        case U8X8_MSG_BYTE_SEND:
            HAL_I2C_Master_Transmit(...);
            break;
        case U8X8_MSG_BYTE_INIT:
            // 初始化I2C
            break;
        // ...
    }
    return 1;
}
```

## 使用方法

### 1. 创建显示器对象

```c
#include "u8g2.h"

u8g2_t u8g2;

void U8g2_Init(void)
{
    // 初始化SSD1306 128×64 I2C
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        &u8g2,
        U8G2_R0,  // 旋转0度
        u8x8_byte_hw_i2c,       // I2C通信回调
        u8x8_gpio_and_delay_template  // GPIO回调
    );

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);  // 唤醒显示
    u8g2_ClearBuffer(&u8g2);
}
```

### 2. 绘制图形

```c
// 清除缓冲区
u8g2_ClearBuffer(&u8g2);

// 绘制字符串
u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
u8g2_DrawStr(&u8g2, 0, 15, "Hello World!");

// 绘制框
u8g2_DrawFrame(&u8g2, 0, 20, 50, 30);

// 绘制实心圆
u8g2_DrawDisc(&u8g2, 80, 40, 15, U8G2_DRAW_ALL);

// 发送缓冲区到显示器
u8g2_SendBuffer(&u8g2);
```

### 3. 使用Unicode字体

```c
u8g2_SetFont(&u8g2, u8g2_font_unifont_t_chinese1);
u8g2_DrawUTF8(&u8g2, 0, 15, "你好世界");
u8g2_SendBuffer(&u8g2);
```

## 缓冲模式

### Full Buffer (_f)
- 全屏缓冲
- RAM占用：1KB (128×64÷8)
- 最快刷新速度

### Page Buffer (_1/_2)
- 分页缓冲
- RAM占用：128字节/页
- 需要多次绘制循环

```c
// Page Buffer示例
u8g2_FirstPage(&u8g2);
do {
    // 绘制内容
    u8g2_DrawStr(&u8g2, 0, 15, "Page Mode");
} while (u8g2_NextPage(&u8g2));
```

## API概览

### 初始化
- `u8g2_Setup_*()` - 设置显示器
- `u8g2_InitDisplay()` - 初始化硬件
- `u8g2_SetPowerSave()` - 电源管理

### 缓冲区管理
- `u8g2_ClearBuffer()` - 清空缓冲
- `u8g2_SendBuffer()` - 发送到显示器
- `u8g2_FirstPage()`/`NextPage()` - 页模式

### 绘图函数
- `u8g2_DrawPixel()` - 画点
- `u8g2_DrawLine()` - 画线
- `u8g2_DrawBox()` - 实心矩形
- `u8g2_DrawFrame()` - 空心矩形
- `u8g2_DrawCircle()` - 空心圆
- `u8g2_DrawDisc()` - 实心圆
- `u8g2_DrawTriangle()` - 三角形

### 文本显示
- `u8g2_SetFont()` - 设置字体
- `u8g2_DrawStr()` - ASCII字符串
- `u8g2_DrawUTF8()` - UTF-8字符串
- `u8g2_DrawGlyph()` - 单个字符

### 字体选择

U8g2内置60+字体族：

```c
// 小字体
u8g2_font_5x7_tr
u8g2_font_6x10_tr

// 中等字体
u8g2_font_ncenB08_tr
u8g2_font_ncenB10_tr

// 大字体
u8g2_font_ncenB14_tr
u8g2_font_ncenB18_tr

// 中文字体
u8g2_font_wqy12_t_chinese1
u8g2_font_wqy15_t_chinese2
u8g2_font_unifont_t_chinese1
```

## MUI菜单系统

U8g2包含完整的菜单界面库：

```c
#include "mui.h"

// 定义菜单
muif_t muif_list[] = {
    MUIF_U8G2_LABEL(0, "Main Menu"),
    MUIF_BUTTON("BTN", btn_callback),
    // ...
};

// 创建MUI对象
mui_t ui;
mui_Init(&ui, &u8g2, fds, muif_list, muif_list_count);

// 事件处理
mui_NextField(&ui);
mui_PrevField(&ui);
mui_SendSelect(&ui);
```

## 典型应用

### 信息面板
```c
void Display_Status(void)
{
    u8g2_ClearBuffer(&u8g2);

    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(&u8g2, 0, 15, "Temperature:");

    char temp_str[10];
    sprintf(temp_str, "%.1f C", temperature);
    u8g2_DrawStr(&u8g2, 0, 30, temp_str);

    u8g2_SendBuffer(&u8g2);
}
```

### 进度条
```c
void Display_Progress(uint8_t percent)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawFrame(&u8g2, 0, 30, 100, 10);
    u8g2_DrawBox(&u8g2, 2, 32, percent - 4, 6);
    u8g2_SendBuffer(&u8g2);
}
```

### 波形显示
```c
void Display_Waveform(uint8_t *data, uint16_t len)
{
    u8g2_ClearBuffer(&u8g2);
    for (int i = 0; i < len - 1; i++) {
        u8g2_DrawLine(&u8g2, i, data[i], i+1, data[i+1]);
    }
    u8g2_SendBuffer(&u8g2);
}
```

## 配置选项

### 减小字体大小
如果Flash空间不足，可以删除不用的字体数据：

1. 编辑`u8g2_fonts.c`
2. 删除不需要的字体数组
3. 重新编译

### 选择显示器
在`u8g2_d_setup.c`和`u8g2_d_memory.c`中只保留需要的显示器驱动。

## 内存占用

- 代码：~40KB（最小配置）
- 字体数据：37MB（完整）→ 可裁剪到100KB
- RAM：1KB（全缓冲）或128字节（页缓冲）

## 性能优化

1. 使用Full Buffer模式（如果RAM够用）
2. 减少不必要的重绘
3. 使用较小的字体
4. 启用硬件加速（如DMA）

## 注意事项

1. u8g2_fonts.c非常大（37MB），编译慢
2. 建议只保留需要的字体
3. 页缓冲模式需要注意绘制顺序
4. 中文字体占用Flash较大
5. 回调函数必须正确实现

## 参考资源

- [U8g2 GitHub](https://github.com/olikraus/u8g2)
- [U8g2 Wiki](https://github.com/olikraus/u8g2/wiki)
- [字体列表](https://github.com/olikraus/u8g2/wiki/fntlistall)
- [API参考](https://github.com/olikraus/u8g2/wiki/u8g2reference)

## 优势对比

### vs 基础OLED驱动
- ✅ 支持更多显示器
- ✅ 丰富的字体库
- ✅ Unicode支持
- ✅ 更强大的绘图功能
- ❌ 代码和Flash占用更大

### 适用场景
- 需要中文显示
- 需要多种字体
- 需要图形用户界面
- 需要跨平台移植
