#include "lvgl_gui.h"
#include "lv_port_indev.h"
#include "lv_demo_widgets.h"
static const char *TAG = "lvgl_gui";
lv_obj_t *gscr;
lv_obj_t *gimg;
lv_obj_t *glabel;
extern EventGroupHandle_t s_gui_event_group;
SemaphoreHandle_t xGuiSemaphore;

extern uint16_t px;
extern uint16_t py;

lv_img_dsc_t IMG = {
    .header.cf = LV_IMG_CF_TRUE_COLOR,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 800,
    .header.h = 480,
    .data_size = 0,
    .data = NULL,
};

extern esp_err_t esp_lcd_new_panel_hx8369(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

static void increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static void backlight_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000, // Set output frequency at 5 kHz
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PIN_NUM_BK_LIGHT,
        .duty = 0, // Set duty to 0%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

static void sntp_cfg(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp.aliyun.com");
    sntp_init();
    setenv( "TZ", "CST-08", 1 );//设置东八区 北京时间
    tzset();
}

void pic_task(void *pvParameter)
{
    time_t now;
    struct tm *tm_now;
    lv_obj_t * label = lv_label_create(gscr);
    lv_obj_set_style_bg_color(gscr, lv_color_hex(0x000000), LV_PART_MAIN);
    
    static lv_style_t font_style;
	lv_style_init(&font_style);
	lv_style_set_text_font(&font_style, &lv_font_montserrat_48);
    lv_obj_add_style(label, &font_style,LV_PART_MAIN);
    
    lv_obj_align(label, LV_ALIGN_CENTER, 100, 0);
    lv_obj_set_size(label,400,100); 
    
    lv_label_set_recolor(label, true); 
    //sntp_cfg();

    while(1) {
    vTaskDelay(pdMS_TO_TICKS(20));
    //time(&now);
    //tm_now = localtime(&now);
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
    {
        //lv_img_set_src(gimg, &IMG);
        //lv_label_set_text_fmt(label, "#ff0080 %d-%02d-%02d#\n   #00ff80 %02d:%02d:%02d#", tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
        lv_label_set_text_fmt(label, "#ff0080 %d %d", px, py);
        xSemaphoreGive(xGuiSemaphore);
    }
  }
}

static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if(code == LV_EVENT_CLICKED)
    {
        lv_obj_t *label = lv_obj_get_child(btn,0);
        //lv_label_set_text_fmt(label, "%d %d", px, py);
    }
}

void lv_btn_test(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn, 0,0);
    lv_obj_set_size(btn, 800,480);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);

    glabel = lv_label_create(btn);
    lv_label_set_text(glabel, "Btn");
    lv_obj_center(glabel);
}

void guiTask(void *pvParameter)
{
    xGuiSemaphore = xSemaphoreCreateMutex();
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions

    ESP_LOGI(TAG, "Turn off LCD backlight");
    // gpio_set_direction(PIN_NUM_BK_LIGHT, GPIO_MODE_OUTPUT);
    // gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_OFF_LEVEL);
    backlight_ledc_init();

    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = PIN_NUM_DC,
        .wr_gpio_num = PIN_NUM_PCLK,
        .data_gpio_nums = {
            PIN_NUM_DATA0,
            PIN_NUM_DATA1,
            PIN_NUM_DATA2,
            PIN_NUM_DATA3,
            PIN_NUM_DATA4,
            PIN_NUM_DATA5,
            PIN_NUM_DATA6,
            PIN_NUM_DATA7,
        },
        .bus_width = 8,
        .max_transfer_bytes = LCD_V_RES * 100 * sizeof(uint16_t),
        .psram_trans_align = PSRAM_DATA_ALIGNMENT,
        .sram_trans_align = 4,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .on_color_trans_done = notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .flags.swap_color_bytes = true,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;

    ESP_LOGI(TAG, "Install LCD driver of hx8369");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_hx8369(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);

    // Set inversion, x/y coordinate order, x/y mirror according to your LCD module spec
    // esp_lcd_panel_invert_color(panel_handle, false);
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, false);

    // the gap is LCD panel specific, even panels with the same driver IC, can have different gap value
    // esp_lcd_panel_set_gap(panel_handle, 0, 20);

    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    lv_color_t *buf1 = NULL;
    lv_color_t *buf2 = NULL;
    buf1 = heap_caps_aligned_alloc(PSRAM_DATA_ALIGNMENT, LCD_V_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    buf2 = heap_caps_aligned_alloc(PSRAM_DATA_ALIGNMENT, LCD_V_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    //buf1 = heap_caps_malloc(LCD_V_RES * 80 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    //buf2 = heap_caps_malloc(LCD_V_RES * 80 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    assert(buf1);
    assert(buf2);
    ESP_LOGI(TAG, "buf1@%p, buf2@%p", buf1, buf2);
    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_V_RES * 100 );

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    //disp_drv.full_refresh = 1;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    // lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    ESP_LOGI(TAG, "Turn on LCD backlight");
    // gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 8191));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

    // First to print one frame
    lv_timer_handler();

    ESP_LOGI(TAG, "Display LVGL animation");
/*
    gscr = lv_scr_act();
    gimg = lv_img_create(gscr);
    lv_obj_set_align(gimg, LV_ALIGN_CENTER);
 */
    lv_port_indev_init();
    //lv_btn_test();
   // xTaskCreatePinnedToCore(pic_task, "pic", 1024 * 4, NULL, 5, NULL, 1);
   lv_demo_widgets();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        //if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
        {
            lv_timer_handler();
            //xSemaphoreGive(xGuiSemaphore);
        }
    }
}
