#include <stdio.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include "lvgl_gui.h"

#include "esp_wifi.h"

#include <string.h>

#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "lwip/sockets.h"

static const char *TAG = "main";
uint8_t *databuff;
extern lv_img_dsc_t IMG;
/*
static esp_err_t user_init_sdcard()
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;
    const char mount_point[] = "/sdcard";
    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.

    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    slot_config.width = 4;

    // On chips where the GPIOs used for SD card can be configured, set them in
    // the slot_config structure:
    slot_config.clk = 47;
    slot_config.cmd = 21;
    slot_config.d0 = 48;
    slot_config.d1 = 45;
    slot_config.d2 = 13;
    slot_config.d3 = 14;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                          "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    return ret;
}
*/
static void print_rtos_status(void *pvParameters)
{
    // char buf[1024];

    while (1)
    {
        size_t size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        size_t size_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        ESP_LOGW(TAG, "%d, %d", size, size_internal);

        // vTaskList(buf);
        // ESP_LOGI(TAG, "\n%s", buf);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}


#define DEFAULT_SCAN_LIST_SIZE 10


static void print_auth_mode(int authmode)
{
    switch (authmode) {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    default:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
    switch (pairwise_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    default:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }

    switch (group_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    default:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }
}

#define PIC_SIZE 852800

static int get_pic(void)
{
    int err = 0;
    int sockfd;
    int len = 0;

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
        len += recv(sockfd, databuff+len, 1024, 0);
    }

    ESP_LOGI(TAG,"len:%d",len);
    close(sockfd);

    IMG.data_size = PIC_SIZE;
    if (IMG.data != NULL)free(IMG.data);
    IMG.data = databuff;  
    
    xTaskCreatePinnedToCore(pic_task, "pic", 1024 * 4, NULL, 5, NULL, 1);
    
    return err;

}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI(TAG,"event_base:%s, event_id:%d\r\n",event_base, (int)event_id);
    wifi_event_ap_staconnected_t *wifi_event_data;
    if (event_base == WIFI_EVENT){
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:                  //STA模式启动
                esp_wifi_connect();
                /* code */
                break;
            case WIFI_EVENT_STA_STOP:                   //STA模式关闭
                /* code */
                break;
            case WIFI_EVENT_STA_DISCONNECTED:           //STA模式断开连接
                /* code */
                break;
            case WIFI_EVENT_AP_START:                   //AP模式启动
                /* code */
                break;
            case WIFI_EVENT_AP_STOP:                    //AP模式关闭
                /* code */
                break;
            case WIFI_EVENT_AP_STACONNECTED:            //一台设备连接到esp32
                wifi_event_ap_staconnected_t *AP_STACONNECTED_EVENT_DATA = (wifi_event_ap_staconnected_t *)event_data;  //获取事件信息
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:         //一台设备断开与esp32的连接
                wifi_event_ap_stadisconnected_t *AP_STADISCONNECTED_EVENT_DATA = (wifi_event_ap_stadisconnected_t *)event_data;  //获取事件信息
                break;
            default:
                break;
        }
    }else if(event_base == IP_EVENT){
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:                       //esp32从路由器获取到ip
            ESP_LOGI(TAG,"get ip");
            get_pic();
            /* code */
            break;
        case IP_EVENT_STA_LOST_IP:                      //esp32失去ip
            /* code */
            break;
        case IP_EVENT_AP_STAIPASSIGNED:                 //esp32给设备分配了ip
            /* code */
            break;
        default:
            break;
        }
    }
}

wifi_config_t sta_config = {
  .sta = {  
     .ssid = "S20",
     .password = "12345678",
     .bssid_set = false,
     .scan_method = WIFI_ALL_CHANNEL_SCAN,
  }
};

static void wifi_p(void *pvParameters)
{
    // char buf[1024];
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );


    ESP_ERROR_CHECK(esp_netif_init());
    
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_config(WIFI_IF_STA,&sta_config);
    /*
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        print_auth_mode(ap_info[i].authmode);
        if (ap_info[i].authmode != WIFI_AUTH_WEP) {
            print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
        }
        ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
    }
*/
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1500));
    }    

}

void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    //xTaskCreatePinnedToCore(print_rtos_status, "print_rtos_status", 1024 * 4, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(wifi_p, "wifi_p", 1024 * 4, NULL, 3, NULL, 1);
    //ESP_ERROR_CHECK(user_init_sdcard());
    xTaskCreatePinnedToCore(guiTask, "guiTask", 1024 * 6, NULL, 5, NULL, 1);
    //xTaskCreatePinnedToCore(pic_task, "pic", 1024 * 4, NULL, 5, NULL, 1);

}
