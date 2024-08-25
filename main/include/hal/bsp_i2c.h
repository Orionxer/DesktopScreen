#ifndef _BSP_I2C_H
#define _BSP_I2C_H

#include "stdint.h"
#include "esp_err.h"

void touch_i2c_master_init(void);

esp_err_t touch_i2c_write(uint8_t cmd, uint8_t *data_wr, size_t size);
esp_err_t touch_i2c_write_read(uint8_t cmd, uint8_t *data_rd, size_t size);

#endif // _BSP_I2C_H