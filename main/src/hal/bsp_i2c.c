#include "bsp_i2c.h"
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "debug_log.h"

// TODO 调试通过后迁移至头文件
#define PORT_NUMBER -1
#define I2C_MASTER_SCL_IO 32
#define I2C_MASTER_SDA_IO 33
#define I2C_MASTER_FREQ_HZ 100000        /*!< I2C master clock frequency */
#define TP_SLAVE_ADDR 0x38                     /*!< ESP32 slave address, you can set any 7bit value */

i2c_master_dev_handle_t dev_handle;

void touch_i2c_master_init(void)
{
    DBG_LOGI("Touch I2C Init");

    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = PORT_NUMBER,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TP_SLAVE_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
}

esp_err_t touch_i2c_write(uint8_t cmd, uint8_t *data_wr, size_t size)
{
    // ! 写入多个字节需要修改此函数
    uint8_t write_buf[2] = {cmd, data_wr[0]};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), -1);
}

esp_err_t touch_i2c_write_read(uint8_t cmd, uint8_t *data_rd, size_t size)
{
    uint8_t cmd_buf[1] = {cmd};
    return i2c_master_transmit_receive(dev_handle, cmd_buf, sizeof(cmd_buf), data_rd, size, -1);
}