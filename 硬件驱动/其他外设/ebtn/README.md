# EBTN 按键驱动库

功能完善的按键驱动库，支持独立按键和组合键。

## 特性

- 支持**独立按键**和**组合键**
- 支持多种事件：按下、释放、单击、长按
- 支持**多击检测**（双击、三击等）
- 可配置去抖时间
- 通过回调函数与硬件解耦
- 纯C实现，适用于任何嵌入式平台

## 依赖

- `bit_array.h` - 位数组操作库

## 使用方法

### 1. 定义按键参数

```c
#include "ebtn.h"

// 按键参数: 去抖20ms, 释放去抖10ms, 单击最短50ms, 最长500ms, 多击间隔300ms, 长按周期500ms, 最大连续3次
const ebtn_btn_param_t btn_param = EBTN_PARAMS_INIT(20, 10, 50, 500, 300, 500, 3);
```

### 2. 定义按键数组

```c
// 定义按键ID
enum {
    KEY_ID_1 = 0,
    KEY_ID_2,
    KEY_ID_3,
};

// 静态按键数组
ebtn_btn_t buttons[] = {
    EBTN_BUTTON_INIT(KEY_ID_1, &btn_param),
    EBTN_BUTTON_INIT(KEY_ID_2, &btn_param),
    EBTN_BUTTON_INIT(KEY_ID_3, &btn_param),
};
```

### 3. 实现回调函数

```c
// 获取按键状态回调
uint8_t get_btn_state(ebtn_btn_t *btn)
{
    switch (btn->key_id) {
        case KEY_ID_1: return !HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
        case KEY_ID_2: return !HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);
        case KEY_ID_3: return !HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin);
    }
    return 0;
}

// 按键事件回调
void btn_event_callback(ebtn_btn_t *btn, ebtn_evt_t evt)
{
    switch (evt) {
        case EBTN_EVT_ONPRESS:
            printf("Key %d pressed\n", btn->key_id);
            break;
        case EBTN_EVT_ONRELEASE:
            printf("Key %d released\n", btn->key_id);
            break;
        case EBTN_EVT_ONCLICK:
            printf("Key %d clicked %d times\n", btn->key_id, btn->click_cnt);
            break;
        case EBTN_EVT_KEEPALIVE:
            printf("Key %d held for %d periods\n", btn->key_id, btn->keepalive_cnt);
            break;
    }
}
```

### 4. 初始化和处理

```c
void main(void)
{
    // 初始化按键库
    ebtn_init(buttons, EBTN_ARRAY_SIZE(buttons), NULL, 0, get_btn_state, btn_event_callback);

    while (1) {
        // 周期性调用处理函数（建议1ms调用一次）
        ebtn_process(HAL_GetTick());
    }
}
```

### 5. 组合键示例

```c
// 定义组合键
ebtn_btn_combo_t combo_buttons[] = {
    EBTN_BUTTON_COMBO_INIT(100, &btn_param),  // 组合键ID=100
};

// 初始化时包含组合键
ebtn_init(buttons, EBTN_ARRAY_SIZE(buttons),
          combo_buttons, EBTN_ARRAY_SIZE(combo_buttons),
          get_btn_state, btn_event_callback);

// 绑定组合键成员（KEY_ID_1 + KEY_ID_2 同时按下）
ebtn_combo_btn_add_btn(&combo_buttons[0], KEY_ID_1);
ebtn_combo_btn_add_btn(&combo_buttons[0], KEY_ID_2);

// 启用组合键优先模式（组合键触发时不触发单独按键事件）
ebtn_set_config(EBTN_CFG_COMBO_PRIORITY);
```

## 参数说明

| 参数 | 说明 |
|------|------|
| `time_debounce` | 按下去抖时间(ms) |
| `time_debounce_release` | 释放去抖时间(ms) |
| `time_click_pressed_min` | 有效单击最短按下时间(ms) |
| `time_click_pressed_max` | 有效单击最长按下时间(ms)，超过则不触发单击 |
| `time_click_multi_max` | 多击判定的最大间隔时间(ms) |
| `time_keepalive_period` | 长按保持事件的触发周期(ms) |
| `max_consecutive` | 最大连续点击次数，达到后立即触发onclick |

## 事件说明

| 事件 | 说明 |
|------|------|
| `EBTN_EVT_ONPRESS` | 按键按下（去抖后） |
| `EBTN_EVT_ONRELEASE` | 按键释放（去抖后） |
| `EBTN_EVT_ONCLICK` | 单击事件（释放后或达到max_consecutive时触发） |
| `EBTN_EVT_KEEPALIVE` | 长按保持事件（按住时周期性触发） |
