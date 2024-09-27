#ifndef _BATTERY_H
#define _BATTERY_H

#include <stdint.h>

uint8_t estimate_battery_level(uint32_t adc_voltage);

#endif // _BATTERY_H