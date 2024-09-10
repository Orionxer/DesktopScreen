#ifndef _FLASH_H
#define _FLASH_H

typedef struct
{
    uint8_t saved_flag;
    char ssid[32];
    char password[64];
}stc_wifi_t;

void flash_init(void);
void flash_write_single_wifi(char *ssid, char *password);
uint8_t flash_read_single_wifi(void);
stc_wifi_t get_wifi_info(void);
void flash_erase_all(void);

#endif // _FLASH_H