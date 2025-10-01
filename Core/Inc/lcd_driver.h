#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "main.h"

typedef struct __LCD_Init
{
    /* SPI */
    SPI_HandleTypeDef *hspi;

    /* Chip select */
    uint32_t CS_Pin;
    GPIO_TypeDef *CS_Port;

    /* Physical reset */
    uint32_t RESET_Pin;
    GPIO_TypeDef *RESET_Port;

    /* Data/Command selector */
    uint32_t DC_Pin;
    GPIO_TypeDef *DC_Port;

    uint16_t bg_color;
} LCD_Init;

typedef struct __LCD_Handle
{
    LCD_Init Init;

    uint16_t width;
    uint16_t height;

    /* Functions */

    void (*Clear)(struct __LCD_Handle *LcdHandle);
    void (*DrawPixelAt)(struct __LCD_Handle *LcdHandle, int x, int y,
            uint16_t color);
    void (*SetDrawPos)(struct __LCD_Handle *LcdHandle, int x, int y);
    void (*DrawPixel)(struct __LCD_Handle *LcdHandle, uint16_t color); // Go automaticaly to next pos

    void (*PrintChar)(struct __LCD_Handle *LcdHandle, int x, int y, int c,
            int size, int fcolor, int bcolor);
    void (*PrintString)(struct __LCD_Handle *LcdHandle, int x, int y, char *text,
            int size, int fc, int bc);
    void (*PrintNumber)(struct __LCD_Handle *LcdHandle, int x, int y, long num, int dec,
            int lsize, int fc, int bc);

} LCD_Handle;

#define FONTWIDTH 12
#define FONTHEIGHT 16

//Font 12x16 vert. MSB
extern const char xchar[][24];

#define WHITE       0xFFFF
#define SILVER1     0xC618
#define SILVER2     0xA510
#define BLACK       0x0000
#define GRAY        0x8410
#define LIGHTGRAY  0xC618
#define LIGHTGREEN 0x07E0
#define LIGHTRED   0xF800
#define LIGHTBLUE  0x03FF
#define RED         0xF800
#define MAROON1     0x8000
#define MAROON2     0x7800
#define FUCHSIA     0xF81F
#define PURPLE1     0x8010
#define PURPLE2     0x780F
#define LIME        0x07E0
#define GREEN       0x0400
#define YELLOW      0xFFE0
#define OLIVE1      0x8400
#define OLIVE2      0x7BE0
#define BLUE        0x001F
#define NAVY1       0x0010
#define NAVY2       0x000F
#define AQUA        0x07FF
#define TEAL        0x0410
#define DARKBLUE    0x0002
#define MAGENTA     0xF81F
#define CYAN        0x07FF
#define DARKCYAN    0x03EF
#define ORANGE      0xFCA0
#define BROWN       0x8200
#define VIOLET      0x9199
#define PINK        0xF97F
#define GOLD        0xA508

#define ROW1  2
#define ROW2  ROW1 + 20
#define ROW3  ROW1 + 40
#define ROW4  ROW1 + 60
#define ROW5  ROW1 + 80
#define ROW6  ROW1 + 100
#define ROW7  ROW1 + 120
#define ROW8  ROW1 + 140
#define ROW9  ROW1 + 160
#define ROW10 ROW1 + 180
#define ROW11 ROW1 + 200
#define ROW12 ROW1 + 220
//STRING FUNCTIONS
int int2asc(long, int, char*, int);

#ifdef __cplusplus
}
#endif

#endif // __LCD_DRIVER_H
