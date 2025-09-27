#ifndef __ILI9341_DRIVER_H__
#define __ILI9341_DRIVER_H__

#include "lcd_driver.h"

void ili9341_init(LCD_Handle* LcdHandle);
void ili9341_set_xy(LCD_Handle *LcdHandle, int x, int y);
void ili9341_draw_pixel(LCD_Handle *LcdHandle, uint16_t color);
void ili9341_draw_pixel_at(LCD_Handle *LcdHandle,int x, int y , uint16_t color);
void ili9341_clear(LCD_Handle *LcdHandle);
void ili9341_fill_screen(LCD_Handle *LcdHandle, uint16_t color);

/*
 *  Write character from font set to destination on screen
 */
void ili9341_putchar(LCD_Handle *LcdHandle, int x, int y, int c, int size, int fcolor, int bcolor);

/*
 * Print String to LCD
 */
void ili9341_putstring(LCD_Handle *LcdHandle, int x, int y, char *text, int size, int fc, int bc);

/*
 * Convert a number to a string and print it
 * col, row: Coordinates, Num: int or long to be displayed
 * dec: Set position of decimal separator
 */
void ili9341_putnumber(LCD_Handle *LcdHandle, int x, int y, long num, int dec, int lsize, int fc, int bc);

#endif // __ILI9341_DRIVER_H__
