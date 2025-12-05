# 本文件夹的一些小说明

## 文件目录

WouoUIPage版源代码非常简单(CSource文件夹下)，按功能可以划分为以下几部分

|           代码文件            |                             作用                             |
| :---------------------------: | :----------------------------------------------------------: |
| WouoUI_conf.h/WouoUI_common.h |  配置文件，定义屏幕宽高、字体等参数/内部通用的一些函数和宏   |
|        WouoUI_font.c/h        |                        字体文件和数组                        |
|        WouoUI_anim.c/h        |                        非线性动画实现                        |
|        WouoUI_msg.c/h         |                      输入消息队列的实现                      |
|       WouoUI_graph.c/h        |             图形层，提供基本的图形和字符绘制函数             |
|      **WouoUI_page.c/h**      |          **提供了Title\List\Wave三个基本的全页面**           |
|      **WouoUI_win.c/h**       |    **提供了Msg\Conf\Val\Spin\ListWin五种基本的弹窗页面**     |
|        **WouoUI.c/h**         |                 **核心状态机和UI调度的代码**                 |
|        WouoUI_user.c/h        | 用户UI文件，内实现了一个使用WouoUIPage版提供接口实现的一个UI的样例 |

## 小白应用教程 (应用层/逻辑层)

本教程旨在帮助已经完成 WouoUI_Page 库底层移植的用户快速上手应用层开发。

### 1. 基本概念

*   **页面 (Page)**: UI 的基本单元，可以是全屏页面（如 `TitlePage`, `ListPage`, `WavePage`）或弹窗页面（如 `MsgWin`, `ConfWin`, `ValWin`, `SpinWin`, `ListWin`）。每个页面都有自己的类型 (`PageType`) 和处理逻辑。
*   **选项 (Option)**: 页面上可交互的元素，通常定义为一个 `Option` 结构体数组。`Option` 包含文本 (`text`)、内容 (`content`)、关联数值 (`val`) 等信息。`text` 字符串的第一个字符有特殊含义，决定了选项的功能（例如 `+` 跳转页面, `~` 打开滑动数值弹窗, `@` 二值选框）。
*   **消息 (InputMsg)**: 用户的输入操作，如 `msg_up`, `msg_down`, `msg_left`, `msg_right`, `msg_click`, `msg_return`。UI 库会自动捕获这些消息并传递给当前页面的回调函数。
*   **回调函数 (CallBackFunc)**: 每个页面可以绑定一个回调函数。当该页面接收到用户输入消息时，这个函数会被调用。你可以在回调函数中编写页面的具体逻辑，例如根据用户操作跳转到其他页面、修改选项的值、弹出窗口等。回调函数原型为 `bool CallBackFunc(const Page *cur_page_addr, InputMsg msg)`。

### 2. 初始化 WouoUI

在你的应用程序初始化部分，需要执行以下步骤：

```c
#include "WouoUI.h"
#include "WouoUI_user.h" // 包含你定义的用户 UI 文件

// 在你的硬件初始化之后调用
void Application_Init() {
    // ... 其他硬件初始化代码 ...

    // 1. 选择默认 UI 配置 (可选，如果使用默认配置)
    WouoUI_SelectDefaultUI();

    // 2. 绑定屏幕刷新函数 (你需要实现这个函数)
    //    这个函数的作用是将 WouoUI 渲染好的缓冲区数据显示到你的屏幕上
    WouoUI_AttachSendBuffFun(Your_SendScreenBuff_Function);

    // 3. 清空屏幕缓冲区并首次发送 (可选，用于清屏)
    WouoUI_BuffClear();
    WouoUI_BuffSend();

    // 4. 初始化你定义的所有 UI 页面 (重要！)
    TestUI_Init(); // 调用 WouoUI_user.c 中定义的初始化函数

    // 5. 设置画笔颜色 (可选)
    WouoUI_GraphSetPenColor(1); // 通常 1 代表前景色
}

// 你需要实现的屏幕刷新函数示例
// 参数 buff 是 WouoUI 渲染好的数据缓冲区
// 参数 size 是缓冲区大小
void Your_SendScreenBuff_Function(uint8_t* buff, uint16_t size) {
    // 在这里添加将 buff 数据发送到你的显示屏的代码
    // 例如： spi_send_data(buff, size);
    //       i2c_send_data(buff, size);
    //       lcd_draw_bitmap(0, 0, WOUOUI_BUFF_WIDTH, WOUOUI_BUFF_HEIGHT, buff);
}
```

**注意**: `Your_SendScreenBuff_Function` 的具体实现取决于你的硬件平台和显示驱动。

### 3. 创建页面

以创建一个简单的列表页面为例：

```c
// 在 WouoUI_user.h 或 WouoUI_user.c 中定义页面对象和选项

#include "WouoUI.h"

// 1. 定义页面对象
ListPage my_list_page;

// 2. 定义页面的选项数组
Option my_list_options[] = {
    {.text = (char*)"- My List Page"}, // 标题项，无功能
    {.text = (char*)"! Option 1"},     // 点击弹出消息
    {.text = (char*)"! Option 2"},     // 点击弹出消息
    {.text = (char*)"~ Adjust Value", .val = 50}, // 点击打开数值调节弹窗
    {.text = (char*)"+ Go to Main"},   // 点击跳转到主页面 (假设 main_page 已定义)
};
#define MY_LIST_OPTION_NUM (sizeof(my_list_options) / sizeof(Option))

// 3. 定义页面的回调函数 (可选但常用)
bool MyListPage_CallBack(const Page *cur_page_addr, InputMsg msg) {
    if (msg == msg_click) {
        Option* selected_option = WouoUI_ListTitlePageGetSelectOpt(cur_page_addr); // 获取当前选中的选项

        // 根据选项文本或其他标识执行不同操作
        if (strcmp(selected_option->text, "! Option 1") == 0) {
            // 设置并跳转到消息弹窗 (假设 common_msg_page 已定义并初始化)
            WouoUI_MsgWinPageSetContent(&common_msg_page, (char*)"You clicked Option 1!");
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_msg_page);
        } else if (strcmp(selected_option->text, "! Option 2") == 0) {
            WouoUI_MsgWinPageSetContent(&common_msg_page, (char*)"You clicked Option 2!");
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_msg_page);
        } else if (strcmp(selected_option->text, "~ Adjust Value") == 0) {
            // 设置并跳转到数值弹窗 (假设 common_val_page 已定义并初始化)
            // 这里可以设置数值范围、步长等
            WouoUI_ValWinPageSetMinStepMax(&common_val_page, 0, 10, 100);
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_val_page);
        } else if (strcmp(selected_option->text, "+ Go to Main") == 0) {
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &main_page); // 跳转回主页面
        }
    } else if (msg == msg_return) {
        // 处理返回键，例如返回上一级页面
        WouoUI_JumpToPage((PageAddr)cur_page_addr, &main_page); // 假设返回到主页面
        return true; // 返回 true 表示消息已被处理，UI 不再自动处理返回
    }
    return false; // 返回 false 表示消息未完全处理，UI 会继续执行默认操作 (例如列表页面的上下移动)
}

// 4. 在 TestUI_Init() 函数中初始化页面
void TestUI_Init() {
    // ... 其他页面的初始化 ...

    WouoUI_ListPageInit(&my_list_page, MY_LIST_OPTION_NUM, my_list_options, Setting_none, MyListPage_CallBack);

    // 初始化共用的弹窗页面 (通常也在这里完成)
    WouoUI_MsgWinPageInit(&common_msg_page, NULL, false, 2, NULL); // 消息弹窗
    WouoUI_ValWinPageInit(&common_val_page, NULL, 0, 0, 100, 1, true, true, NULL); // 数值弹窗
    // ... 初始化其他可能用到的弹窗 ...

    // ... 设置主页 ...
    p_cur_ui->home_page = &main_page; // 设置哪个页面是主页
}
```

**关键点**:

*   `Option` 数组定义了页面的内容和交互。
*   `WouoUI_ListPageInit` (或其他页面类型的 `Init` 函数) 用于创建页面实例，需要传入选项数量、选项数组、页面设置和回调函数。
*   回调函数是处理页面逻辑的核心。

### 4. 页面跳转

使用 `WouoUI_JumpToPage(PageAddr self_page, PageAddr terminate_page)` 函数进行页面切换。

*   `self_page`: 当前页面的地址，通常传入回调函数的 `cur_page_addr`。
*   `terminate_page`: 要跳转到的目标页面的地址。

```c
// 在回调函数中跳转
WouoUI_JumpToPage((PageAddr)cur_page_addr, &target_page);
```

### 5. 处理用户输入

*   **自动处理**: 默认情况下，WouoUI 会自动处理大部分页面的上、下、左、右移动逻辑。
*   **回调函数处理**: 你可以在回调函数中捕获 `msg_click` 和 `msg_return` 等消息，执行自定义操作。
*   **获取选中项**: 使用 `WouoUI_ListTitlePageGetSelectOpt(cur_page_addr)` 获取列表或磁贴页面当前选中的 `Option` 指针。对于其他页面类型，可能有类似的函数。

### 6. 使用弹窗

弹窗也是一种特殊的页面。

1.  **定义弹窗页面对象**: 和普通页面类似，例如 `MsgWin common_msg_page;`。
2.  **初始化弹窗页面**: 在 `TestUI_Init()` 中调用对应的初始化函数，如 `WouoUI_MsgWinPageInit()`, `WouoUI_ConfWinPageInit()` 等。可以设置默认值、回调函数等。
3.  **触发弹窗**: 在需要弹出窗口的地方 (通常是某个页面的回调函数中)，调用 `WouoUI_JumpToPage()` 跳转到弹窗页面。
4.  **配置弹窗内容/行为 (可选)**: 在跳转前，可以调用特定函数设置弹窗内容，例如 `WouoUI_MsgWinPageSetContent()` 设置消息内容，`WouoUI_ValWinPageSetMinStepMax()` 设置数值范围。
5.  **处理弹窗结果**:
    *   对于 `ConfWin` (确认弹窗)，可以在其回调函数中检查 `conf_ret` 成员获取用户的选择 (Yes/No)。
    *   对于 `ValWin` 或 `SpinWin`，可以在其回调函数中检查 `val` 成员获取用户调节后的值，或者在初始化时设置 `auto_set_bg_opt` 为 `true`，让弹窗自动将结果写回触发它的那个选项的 `val` 值。
    *   对于 `ListWin`，可以在其回调函数中检查 `sel_str_index` 获取用户选择的列表项索引。

### 7. 配置 UI 参数 (进阶)

可以通过修改全局变量 `g_default_ui_para` 来调整 UI 的一些视觉和行为参数，例如动画速度、列表循环模式、弹窗背景模糊程度等。

```c
// 示例：减慢列表动画速度
g_default_ui_para.ani_param[LIST_ANI] = 300; // 数值越大越慢

// 示例：开启磁贴页面循环模式
g_default_ui_para.loop_param[TILE_LOOP] = 1; // 1 表示开启
```

修改这些参数通常在 `TestUI_Init()` 函数中进行，放在页面初始化之前。

### 8. 主循环集成

在你的应用程序主循环 (例如 `while(1)`) 中，你需要：

1.  **获取用户输入**: 检测按键或其他输入设备，并将用户的操作转换为 `InputMsg`。
2.  **发送消息**: 将获取到的 `InputMsg` 发送到 WouoUI 的消息队列中。
3.  **处理 UI**: 调用 `WouoUI_Proc()` 函数，传入两次调用之间的时间间隔 (毫秒)，驱动 UI 状态机运行、处理消息、执行动画和渲染。

```c
#include "WouoUI.h"

uint32_t last_tick = 0; // 用于计算时间间隔

int main(void) {
    Application_Init(); // 执行之前的初始化

    while (1) {
        // 1. 获取用户输入 (你需要实现这部分)
        InputMsg current_msg = Get_User_Input(); // 返回 msg_none 如果无输入

        // 2. 发送消息 (如果有输入)
        if (current_msg != msg_none) {
            WouoUI_SendMsg(current_msg);
        }

        // 3. 计算时间间隔
        uint32_t current_tick = Your_GetTick_Function(); // 获取当前系统时间 (毫秒)
        uint16_t delta_time = current_tick - last_tick;
        last_tick = current_tick;

        // 4. 处理 UI
        WouoUI_Proc(delta_time);

        // ... 其他需要在主循环中执行的任务 ...
        // 例如： Your_LowPower_Handler();
    }
}

// 你需要实现的获取用户输入的函数示例
InputMsg Get_User_Input() {
    if (Is_KeyUp_Pressed()) return msg_up;
    if (Is_KeyDown_Pressed()) return msg_down;
    if (Is_KeyLeft_Pressed()) return msg_left;
    if (Is_KeyRight_Pressed()) return msg_right;
    if (Is_KeyEnter_Pressed()) return msg_click;
    if (Is_KeyBack_Pressed()) return msg_return;
    return msg_none; // 没有按键按下
}

// 你需要实现的获取系统 Tick 的函数示例
uint32_t Your_GetTick_Function() {
    // 返回 HAL_GetTick() 或其他获取毫秒级时间戳的函数
    return HAL_GetTick();
}
```

### 9. 简单示例回顾

结合以上步骤，一个最简单的应用流程：

1.  **硬件和 WouoUI 初始化**: `Application_Init()` 中完成。
2.  **定义页面和选项**: 在 `WouoUI_user.c/.h` 中定义 `main_page`、`my_list_page` 等，以及它们的 `Option` 数组和回调函数。
3.  **初始化页面**: 在 `TestUI_Init()` 中调用 `WouoUI_TitlePageInit()`, `WouoUI_ListPageInit()` 等。
4.  **主循环**:
    *   检测按键输入 -> `Get_User_Input()`
    *   发送消息 -> `WouoUI_SendMsg()`
    *   处理 UI -> `WouoUI_Proc()`
5.  **页面逻辑**: 在页面的回调函数中处理 `msg_click` 和 `msg_return`，使用 `WouoUI_JumpToPage()` 进行页面切换或弹出窗口。

希望这份教程能帮助你快速上手 WouoUI_Page 的应用层开发！如有疑问，请参考 `WouoUI_user.c` 中的示例代码，它展示了更复杂的用法。

