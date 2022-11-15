#include <stdio.h>
#include <esp_log.h>
#include "driver/i2c.h"
#include "gt9xx.h"
#include "myi2c.h"
#include "gt911.h"
static const char *TAG = "DEMO";

void gt911(void *pvParameter) {
    i2c_master_init();
    GT9xx_Class *a1=new GT9xx_Class();
    a1->begin(0);

    uint16_t x=0;
    uint16_t y=0;
    int bb=0;

    while (true){
        vTaskDelay(pdMS_TO_TICKS(20));
        bb=a1->scanPoint();
        //for (size_t i = 0; i < 4; i++)
        {
            a1->getPoint(x,y,0);
            ESP_LOGE(TAG, "%d/%d--%d  %d ",0,bb,x,y);
        }
        

    }


}
