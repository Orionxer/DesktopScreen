#ifndef _SCREEN_H
#define _SCREEN_H

void screen_init(void);

void EPD_Dis_Part(unsigned int x_start, unsigned int y_start, const unsigned char *datas, unsigned int PART_COLUMN, unsigned int PART_LINE);

#endif // _SCREEN_H
