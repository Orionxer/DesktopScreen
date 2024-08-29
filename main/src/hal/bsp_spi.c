#include <string.h>
#include "driver/spi_master.h"
#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "debug_log.h"

#define PIN_NUM_MISO 23
#define PIN_NUM_MOSI 26
#define PIN_NUM_CLK  25
#define PIN_NUM_CS   27

spi_device_handle_t spi;

void screen_spi_master_init(void)
{
    DBG_LOGD("SPI Initializing");
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO, // MISO信号线
        .mosi_io_num = PIN_NUM_MOSI, // MOSI信号线
        .sclk_io_num = PIN_NUM_CLK,  // SCLK信号线
        .quadwp_io_num = -1,         // WP信号线，专用于QSPI的D2
        .quadhd_io_num = -1,         // HD信号线，专用于QSPI的D3
        .max_transfer_sz = 64 * 8,   // 最大传输数据大小

    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 15 * 1000 * 1000, // Clock out at 26 MHz
        .mode = 0,                          // SPI mode 0
        .queue_size = 7,                    // We want to be able to queue 7 transactions at a time
        // .pre_cb=spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    // Initialize the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);
    ESP_ERROR_CHECK(ret);
    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

void spi_send_cmd(uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    ds_gpio_set_screen_dc(0);
    ds_gpio_set_screen_cs(0);
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    // t.flags=SPI_TRANS_USE_TXDATA;
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    ds_gpio_set_screen_cs(1);
    assert(ret==ESP_OK);            //Should have had no issues.
}

void spi_send_data(uint8_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    ds_gpio_set_screen_dc(1);
    ds_gpio_set_screen_cs(0);
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=&data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    ds_gpio_set_screen_cs(1);
    assert(ret==ESP_OK);            //Should have had no issues.
}

// TODO SPI连续发送字节