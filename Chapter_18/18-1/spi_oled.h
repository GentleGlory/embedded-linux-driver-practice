#ifndef _SPI_OLED_H
#define _SPI_OLED_H

#define X_WIDTH    128
#define Y_WIDTH    64

typedef struct oled_display_struct
{
    u8 x;
    u8 y;
    u32 length;
    u8 display_buffer[];
}oled_display_struct;


#endif