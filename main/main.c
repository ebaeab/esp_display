#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/gpio.h"
#include "lvgl_gui.h"
#include "smart.h"
#include "esp_wifi.h"
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "gt911.h"
//static const char *TAG = "main";
uint8_t *databuff;
extern lv_img_dsc_t IMG;

#define PIC_SIZE 852800

int get_pic(void)
{
    int err = 0;
    //int sockfd;
    //int len = 0;
/*
    sockfd = socket(AF_INET,SOCK_STREAM,0);

    //databuff = heap_caps_malloc(PIC_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    databuff = (uint8_t *)malloc(PIC_SIZE);
    struct sockaddr_in  server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8090);
    server_addr.sin_addr.s_addr = inet_addr("124.221.129.178");
    connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    while(len < PIC_SIZE)
    {
        len += recv(sockfd, databuff+len, 4096, 0);
    }

    ESP_LOGI(TAG,"len:%d",len);
    close(sockfd);

    IMG.data_size = PIC_SIZE;
    if (IMG.data != NULL)free((void *)IMG.data);
    IMG.data = databuff;  
    */
    xTaskCreatePinnedToCore(pic_task, "pic", 1024 * 4, NULL, 5, NULL, 1);
    
    return err;

}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    ESP_ERROR_CHECK( ret );
    initialise_wifi();
    //xTaskCreatePinnedToCore(gt911, "gt911", 1024 * 6, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(guiTask, "guiTask", 1024 * 6, NULL, 5, NULL, 1);
    //gpio_init();
}
