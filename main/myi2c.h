
#ifndef MYI2C_H
#define MYI2C_H

#include "driver/i2c.h"

#define FT5206_VENDID                   (0x11)
#define FT6206_CHIPID                   (0x06)
#define FT6236_CHIPID                   (0x36)
#define FT6236U_CHIPID                  (0x64)
#define FT5206U_CHIPID                  (0x64)
#define I2C_MASTER_SCL_IO          14
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000
#define SENSOR_ADDR                 0x5D
#ifdef __cplusplus
extern "C"
{
#endif

esp_err_t i2c_master_init(void);

#ifdef __cplusplus
}
#endif
#endif //MYI2C_H
