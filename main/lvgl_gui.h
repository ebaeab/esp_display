#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "driver/ledc.h"

#include "demos/lv_demos.h"

// PCLK frequency can't go too high as the limitation of PSRAM bandwidth
#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)

#define LCD_BK_LIGHT_ON_LEVEL 1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL

#define PIN_NUM_DATA0 46
#define PIN_NUM_DATA1 3
#define PIN_NUM_DATA2 8
#define PIN_NUM_DATA3 18
#define PIN_NUM_DATA4 17
#define PIN_NUM_DATA5 16
#define PIN_NUM_DATA6 15
#define PIN_NUM_DATA7 7

#define PIN_NUM_PCLK 10
#define PIN_NUM_CS 12
#define PIN_NUM_DC 11
#define PIN_NUM_RST 9
#define PIN_NUM_BK_LIGHT 6

// The pixel number in horizontal and vertical
#define LCD_H_RES 800
#define LCD_V_RES 480
// Bit number used to represent command and parameter
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LVGL_TICK_PERIOD_MS 2

// Supported alignment: 16, 32, 64. A higher alignment can enables higher burst transfer size, thus a higher i80 bus throughput.
#define PSRAM_DATA_ALIGNMENT 64

void guiTask(void *pvParameter);
void pic_task(void *pvParameter);