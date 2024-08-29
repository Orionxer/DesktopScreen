#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "screen.h"
#include "debug_log.h"

void screen_init(void)
{
    DBG_LOGD("Screen Initializing");
    screen_gpio_init();
    screen_spi_master_init();
}