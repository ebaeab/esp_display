
#include "gt9xx.h"
#include "driver/i2c.h"

static const char *TAG = "GT9XX";

#define GT9XX_COORDINATE            (0x814E)
#define GT9XX_CLEARBUF              (0x814E)
#define GT9XX_CONFIG                (0x8047)
#define GT9XX_COMMAND               (0x8040)
#define GT9XX_PRODUCT_ID            (0x8140)
#define GT9XX_VENDOR_ID             (0x814A)
#define GT9XX_CONFIG_VERSION        (0x8047)
#define GT9XX_CONFIG_CHECKSUM       (0x80FF)
#define GT9XX_FIRMWARE_VERSION      (0x8144)


bool GT9xx_Class::probe(void)
{
  /* uint8_t config0[] = {
        0x00,0xE0,0x01,0x20,0x03,0x05,0x04,0x10,0x01,0xC8,
        0x28,0x0F,0x50,0x32,0x03,0x05,0x00,0x00,0x00,0x00,
        0x11,0x11,0x05,0x18,0x1A,0x1E,0x14,0x88,0x29,0x0A,
        0x52,0x50,0x40,0x04,0x00,0x00,0x00,0x1A,0x32,0x1C,
        0x46,0x09,0x00,0x0F,0x00,0x2A,0xFF,0x7F,0x19,0x50,
        0x32,0x3C,0x64,0x94,0xD5,0x02,0x07,0x00,0x00,0x04,
        0x9F,0x3F,0x00,0x90,0x46,0x00,0x84,0x4D,0x00,0x79,
        0x55,0x00,0x6D,0x5F,0x00,0x6D,0x00,0x00,0x00,0x00,
        0xF0,0x4A,0x3A,0xFF,0xFF,0x27,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x14,0x12,0x10,0x0E,0x0C,0x0A,0x08,0x06,
        0x04,0x02,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0x22,0x21,0x20,0x1F,0x1E,0x1D,0x1C,0x18,
        0x16,0x12,0x10,0x0F,0x08,0x06,0x04,0x02,0x00,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0x0B,0x01
        };
*/ 
    softReset();

    uint8_t buffer[5] = {0};
    readBytes(GT9XX_PRODUCT_ID, buffer, 3);
    buffer[3] = readRegister(GT9XX_CONFIG_VERSION);

    ESP_LOGI(TAG, "TouchPad_ID:%c,%c,%c\r\nTouchPad_Config_Version:%2x\r\n", buffer[0], buffer[1], buffer[2], buffer[3]);
    if(buffer[0] == '9'){
        initialization = true;
    }else{
        initialization = false;
        return false;
    }
/*
    uint8_t check_sum = 0;
    for (int i = 0; i < (sizeof(config0) - 2); i++) {
        check_sum += config0[i];
    }
    config0[184] = (~check_sum) + 1;
    writeBytes(GT9XX_CONFIG, config0, sizeof(config0));
*/
    return true;
}

uint8_t GT9xx_Class::readRegister(uint16_t reg)
{
    uint8_t value;
    (void)readBytes(reg, &value, 1);
    return value;
}

void GT9xx_Class::writeRegister(uint16_t reg, uint8_t value)
{
    (void)writeBytes(reg, &value, 1);
}

bool GT9xx_Class::readBytes( uint16_t reg, uint8_t *data, int nbytes)
{
    if (_readCallbackFunc != nullptr) {
        return _readCallbackFunc(_address, reg, data, nbytes);
    }
    uint8_t a1[2];
    a1[0]=reg>>8;
    a1[1]=reg&0xff;
    i2c_master_write_read_device(I2C_NUM_0,0x5D,a1,2,data,nbytes,1000/portTICK_PERIOD_MS);
    return true;
}

bool GT9xx_Class::writeBytes(uint16_t reg, uint8_t *data, int nbytes)
{
    if (_writeCallbackFunc != nullptr) {
        return _writeCallbackFunc(_address, reg, data, nbytes);
    }
    uint8_t a1[2+nbytes];
    a1[0]=reg>>8;
    a1[1]=reg&0xff;
    for(int k=0;k<nbytes;k++){
        a1[2+k]=data[k];
    }
    ESP_ERROR_CHECK(i2c_master_write_to_device(I2C_NUM_0, 0x5D, a1, 2+nbytes, 1000 / portTICK_PERIOD_MS));
    return true;
}

void GT9xx_Class::setPins(int rst, int interrupt)
{
    _rst = rst;
    _interrupt = interrupt;
}

void GT9xx_Class::softReset(void)
{
    writeRegister(GT9XX_COMMAND, 0x01);
}

bool GT9xx_Class::begin(uint8_t addr)
{

    _address = addr;
    _readCallbackFunc = nullptr;
    _writeCallbackFunc = nullptr;
    return probe();
}

bool GT9xx_Class::begin(iic_com_fptr_t read_cb, iic_com_fptr_t write_cb, uint8_t addr)
{
    if (read_cb == nullptr || write_cb == nullptr) {
        return false;
    }
    _readCallbackFunc = read_cb;
    _writeCallbackFunc = write_cb;
    _address = addr;
    return probe();
}

uint8_t GT9xx_Class::scanPoint()
{
    uint8_t point = 0;
    uint8_t buffer[40] = {0};
    if (!readBytes(GT9XX_COORDINATE, buffer, 40)) {
        ESP_LOGE(TAG, "error");
        return false;
    }
    writeRegister(GT9XX_CLEARBUF, 0);

    point = buffer[0] & 0xF;

    if (point == 0) {
        return 0;
    }

    data[0].y = 480 - ((uint16_t)buffer[3] << 8) - buffer[2];
    data[0].x = ((uint16_t)buffer[5] << 8) + buffer[4];
/*
    if (data[0].x > 400)
    {
        data[0].x -= 447;
    }
    else if (data[0].x <= 400)
    {
        data[0].x += 353;
    }
*/

    data[1].y = ((uint16_t)buffer[11] << 8) + buffer[10];
    data[1].x = ((uint16_t)buffer[13] << 8) + buffer[12];

    data[2].y = ((uint16_t)buffer[19] << 8) + buffer[18];
    data[2].x = ((uint16_t)buffer[21] << 8) + buffer[20];

    data[3].y = ((uint16_t)buffer[27] << 8) + buffer[26];
    data[3].x = ((uint16_t)buffer[29] << 8) + buffer[28];

    data[4].y = ((uint16_t)buffer[35] << 8) + buffer[34];
    data[4].x = ((uint16_t)buffer[37] << 8) + buffer[36];

    return point;
}

void GT9xx_Class::getPoint(uint16_t &x, uint16_t &y, uint8_t index)
{
    if (index >= 4)return;
    x = data[index].x;
    y = data[index].y;
}

