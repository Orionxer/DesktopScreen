#ifndef _BSP_SPI_H
#define _BSP_SPI_H

#include <stdint.h>

void screen_spi_master_init(void);
void spi_send_cmd(uint8_t cmd);
void spi_send_data(uint8_t data);

#endif // _BSP_SPI_H