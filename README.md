# esp_display

基于 ESP32-S3 的嵌入式显示项目，驱动 800x480 HX8369A LCD 屏幕，集成 LVGL 图形库和 GT911 触摸屏。

## 核心架构

| 层级 | 组件 | 说明 |
|------|------|------|
| 显示驱动 | `esp_lcd_panel_hx8369.c` | HX8369A LCD 面板驱动，Intel 8080 并行接口（8-bit） |
| 图形库 | `components/lvgl/` | LVGL v8.x，UI 渲染 |
| 触摸输入 | `components/GT9XX/` + `main/gt911demo.cpp` | GT911 电容触摸屏驱动，I2C 通信 |
| 输入设备 | `lv_port_indev.c` | 触摸事件桥接到 LVGL 输入设备接口 |
| GUI 任务 | `lvgl_gui.c` | 初始化 LCD、LVGL，运行 UI |
| I2C 驱动 | `myi2c.c` | I2C 主机初始化（400kHz） |

## 硬件引脚

- LCD 数据总线: GPIO 46, 3, 8, 18, 17, 16, 15, 7（8-bit 并行）
- LCD 控制: PCLK=GPIO10, CS=GPIO12, DC=GPIO11, RST=GPIO9
- 背光: GPIO6（LEDC PWM 调光，5kHz）
- 触摸 I2C: SDA=GPIO21, SCL=GPIO14

## 启动流程

1. 初始化 NVS Flash
2. 创建 `gt911` 任务（触摸扫描，20ms 轮询）
3. 创建 `guiTask` 任务（初始化 LCD + LVGL，运行 UI）

## 目标配置

- 芯片: ESP32-S3
- 分区: nvs + phy_init + 8MB factory app（带 PSRAM）
- 项目名: HX8369A_DEV

## 组件依赖

- `components/lvgl/` — LVGL 图形库（完整源码）
- `components/GT9XX/` — GT9xx 触摸屏驱动（C++ 类封装）
