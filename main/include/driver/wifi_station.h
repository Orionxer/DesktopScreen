#ifndef _WIFI_STATION_H
#define _WIFI_STATION_H

#include <stdint.h>

void wifi_init_station(void);

void wifi_read_rssi(void);

uint8_t wifi_connect(char *ssid, char *password);

void wifi_disconnect(void);

void wifi_test_flash(void);

void wifi_test_smartconfig(void);

uint8_t wifi_read_state(void);

#endif // _WIFI_STATION_H