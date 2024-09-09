#ifndef _FLASH_H
#define _FLASH_H

void flash_init(void);
void flash_write_single_wifi(char *ssid, char *password);
uint8_t flash_read_single_wifi(void);
void flash_erase_all(void);

#endif // _FLASH_H