#ifndef _BSP_GPIO_H
#define _BSP_GPIO_H

#include <stdint.h>

void set_tp_rst_level(uint8_t level);

void touch_gpio_init(void);

void screen_gpio_init(void);
void ds_gpio_set_screen_cs(uint32_t level);
void ds_gpio_set_screen_dc(uint32_t level);
void ds_gpio_set_screen_rst(uint32_t level);
int ds_gpio_get_screen_busy();

#endif // _BSP_GPIO_H