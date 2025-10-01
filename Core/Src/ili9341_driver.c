/*
 * ILI9341_Driver.c
 *
 *  Created on: Sep 27, 2025
 *      Author: Vectem
 */

#include <ili9341_driver.h>
#include <stdlib.h>

#define LCD_CMD   0
#define LCD_DATA  1

static void ili9341_delay(unsigned int time)
{
    for (unsigned int i = 0; i < time; i++)
    {
        for (volatile unsigned int j = 0; j < 2000; j++)
            ;
    }
}

// SPI write data or command to LCD (bit-banging)
void ili9341_send(LCD_Handle *LcdHandle, int dc, uint8_t value)
{

    // DC (Command = 0, Data = 1)
    if (dc == 0)
    {
        HAL_GPIO_WritePin(LcdHandle->Init.DC_Port, LcdHandle->Init.DC_Pin,
                GPIO_PIN_RESET); // Cmd
    }
    else
    {
        HAL_GPIO_WritePin(LcdHandle->Init.DC_Port, LcdHandle->Init.DC_Pin,
                GPIO_PIN_SET);   // Data
    }

    // CS Low
    HAL_GPIO_WritePin(LcdHandle->Init.CS_Port, LcdHandle->Init.CS_Pin,
            GPIO_PIN_RESET);

    // Send 1 byte via SPI hardware
    HAL_SPI_Transmit(LcdHandle->Init.hspi, &value, 1, HAL_MAX_DELAY);

    // CS High
    HAL_GPIO_WritePin(LcdHandle->Init.CS_Port, LcdHandle->Init.CS_Pin,
            GPIO_PIN_SET);

}

void ili9341_reset(LCD_Handle *LcdHandle)
{
    HAL_GPIO_WritePin(LcdHandle->Init.RESET_Port, LcdHandle->Init.RESET_Pin,
            GPIO_PIN_RESET); // 0
    HAL_GPIO_WritePin(LcdHandle->Init.RESET_Port, LcdHandle->Init.RESET_Pin,
            GPIO_PIN_SET);   // 1
    HAL_Delay(5);
}

void ili9341_init(LCD_Handle *LcdHandle)
{

    LcdHandle->width = 320;
    LcdHandle->height = 240;
    LcdHandle->Clear = ili9341_clear;
    LcdHandle->DrawPixelAt = ili9341_draw_pixel_at;
    LcdHandle->PrintChar = ili9341_putchar;
    LcdHandle->PrintString = ili9341_putstring;
    LcdHandle->PrintNumber = ili9341_putnumber;
    LcdHandle->DrawPixel = ili9341_draw_pixel;
    LcdHandle->SetDrawPos = ili9341_set_xy;

    ili9341_reset(LcdHandle);

    ili9341_send(LcdHandle, LCD_CMD, 0xCB);
    ili9341_send(LcdHandle, LCD_DATA, 0x39);
    ili9341_send(LcdHandle, LCD_DATA, 0x2C);
    ili9341_send(LcdHandle, LCD_DATA, 0x00);
    ili9341_send(LcdHandle, LCD_DATA, 0x34);
    ili9341_send(LcdHandle, LCD_DATA, 0x02);

    ili9341_send(LcdHandle, LCD_CMD, 0xCF);
    ili9341_send(LcdHandle, LCD_DATA, 0x00);
    ili9341_send(LcdHandle, LCD_DATA, 0XC1);
    ili9341_send(LcdHandle, LCD_DATA, 0X30);

    ili9341_send(LcdHandle, LCD_CMD, 0xE8);
    ili9341_send(LcdHandle, LCD_DATA, 0x85);
    ili9341_send(LcdHandle, LCD_DATA, 0x00);
    ili9341_send(LcdHandle, LCD_DATA, 0x78);

    ili9341_send(LcdHandle, LCD_CMD, 0xEA);
    ili9341_send(LcdHandle, LCD_DATA, 0x00);
    ili9341_send(LcdHandle, LCD_DATA, 0x00);

    ili9341_send(LcdHandle, LCD_CMD, 0xED);
    ili9341_send(LcdHandle, LCD_DATA, 0x64);
    ili9341_send(LcdHandle, LCD_DATA, 0x03);
    ili9341_send(LcdHandle, LCD_DATA, 0X12);
    ili9341_send(LcdHandle, LCD_DATA, 0X81);

    ili9341_send(LcdHandle, LCD_CMD, 0xF7);
    ili9341_send(LcdHandle, LCD_DATA, 0x20);

    ili9341_send(LcdHandle, LCD_CMD, 0xC0); // Power control
    ili9341_send(LcdHandle, LCD_DATA, 0x23); // VRH[5:0]

    ili9341_send(LcdHandle, LCD_CMD, 0xC1); // Power control
    ili9341_send(LcdHandle, LCD_DATA, 0x10); // SAP[2:0];BT[3:0]

    ili9341_send(LcdHandle, LCD_CMD, 0xC5); // VCM control
    ili9341_send(LcdHandle, LCD_DATA, 0x3e);
    ili9341_send(LcdHandle, LCD_DATA, 0x28);

    ili9341_send(LcdHandle, LCD_CMD, 0xC7); // VCM control2
    ili9341_send(LcdHandle, LCD_DATA, 0x86);

    ili9341_send(LcdHandle, LCD_CMD, 0x36); // Memory Access Control
    ili9341_send(LcdHandle, LCD_DATA, 0x88); // C8

    ili9341_send(LcdHandle, LCD_CMD, 0x3A);
    ili9341_send(LcdHandle, LCD_DATA, 0x55);

    ili9341_send(LcdHandle, LCD_CMD, 0xB1);
    ili9341_send(LcdHandle, LCD_DATA, 0x00);
    ili9341_send(LcdHandle, LCD_DATA, 0x18);

    ili9341_send(LcdHandle, LCD_CMD, 0xB6); // Display Function Control
    ili9341_send(LcdHandle, LCD_DATA, 0x08);
    ili9341_send(LcdHandle, LCD_DATA, 0x82);
    ili9341_send(LcdHandle, LCD_DATA, 0x27);

    ili9341_send(LcdHandle, LCD_CMD, 0xF2); // 3Gamma Function Disable
    ili9341_send(LcdHandle, LCD_DATA, 0x00);

    ili9341_send(LcdHandle, LCD_CMD, 0x26); // Gamma curve selected
    ili9341_send(LcdHandle, LCD_DATA, 0x01);

    ili9341_send(LcdHandle, LCD_CMD, 0xE0); // Set Gamma
    ili9341_send(LcdHandle, LCD_DATA, 0x0F);
    ili9341_send(LcdHandle, LCD_DATA, 0x31);
    ili9341_send(LcdHandle, LCD_DATA, 0x2B);
    ili9341_send(LcdHandle, LCD_DATA, 0x0C);
    ili9341_send(LcdHandle, LCD_DATA, 0x0E);
    ili9341_send(LcdHandle, LCD_DATA, 0x08);
    ili9341_send(LcdHandle, LCD_DATA, 0x4E);
    ili9341_send(LcdHandle, LCD_DATA, 0xF1);
    ili9341_send(LcdHandle, LCD_DATA, 0x37);
    ili9341_send(LcdHandle, LCD_DATA, 0x07);
    ili9341_send(LcdHandle, LCD_DATA, 0x10);
    ili9341_send(LcdHandle, LCD_DATA, 0x03);
    ili9341_send(LcdHandle, LCD_DATA, 0x0E);
    ili9341_send(LcdHandle, LCD_DATA, 0x09);
    ili9341_send(LcdHandle, LCD_DATA, 0x00);

    ili9341_send(LcdHandle, LCD_CMD, 0xE1); // Set Gamma
    ili9341_send(LcdHandle, LCD_DATA, 0x00);
    ili9341_send(LcdHandle, LCD_DATA, 0x0E);
    ili9341_send(LcdHandle, LCD_DATA, 0x14);
    ili9341_send(LcdHandle, LCD_DATA, 0x03);
    ili9341_send(LcdHandle, LCD_DATA, 0x11);
    ili9341_send(LcdHandle, LCD_DATA, 0x07);
    ili9341_send(LcdHandle, LCD_DATA, 0x31);
    ili9341_send(LcdHandle, LCD_DATA, 0xC1);
    ili9341_send(LcdHandle, LCD_DATA, 0x48);
    ili9341_send(LcdHandle, LCD_DATA, 0x08);
    ili9341_send(LcdHandle, LCD_DATA, 0x0F);
    ili9341_send(LcdHandle, LCD_DATA, 0x0C);
    ili9341_send(LcdHandle, LCD_DATA, 0x31);
    ili9341_send(LcdHandle, LCD_DATA, 0x36);
    ili9341_send(LcdHandle, LCD_DATA, 0x0F);

    ili9341_send(LcdHandle, LCD_CMD, 0x11); // Sleep out
    ili9341_delay(120);
    ili9341_send(LcdHandle, LCD_CMD, 0x2c);

    ili9341_send(LcdHandle, LCD_CMD, 0x29); // Display on
    ili9341_send(LcdHandle, LCD_CMD, 0x2c);
}

void ili9341_set_xy(LCD_Handle *LcdHandle, int x, int y)
{
    //X
    ili9341_send(LcdHandle, LCD_CMD, 0x2B);
    ili9341_send(LcdHandle, LCD_DATA, x >> 8);
    ili9341_send(LcdHandle, LCD_DATA, x & 0xFF);
    ili9341_send(LcdHandle, LCD_CMD, 0x2c);

    //Y
    ili9341_send(LcdHandle, LCD_CMD, 0x2A);
    ili9341_send(LcdHandle, LCD_DATA, y >> 8);
    ili9341_send(LcdHandle, LCD_DATA, y & 0xFF);
    ili9341_send(LcdHandle, LCD_CMD, 0x2c);
}

void ili9341_draw_pixel(LCD_Handle *LcdHandle, uint16_t color)
{
    ili9341_send(LcdHandle, LCD_DATA, color >> 8);
    ili9341_send(LcdHandle, LCD_DATA, color & 0xFF);
}

void ili9341_draw_pixel_at(LCD_Handle *LcdHandle,int x, int y , uint16_t color)
{
    ili9341_set_xy(LcdHandle, x, y);
    ili9341_draw_pixel(LcdHandle, color);
}


void ili9341_clear(LCD_Handle *LcdHandle)
{
    ili9341_fill_screen(LcdHandle, LcdHandle->Init.bg_color);
}

void ili9341_fill_screen(LCD_Handle *LcdHandle, uint16_t color)
{

    ili9341_set_xy(LcdHandle, 0, 0);

    HAL_GPIO_WritePin(LcdHandle->Init.DC_Port, LcdHandle->Init.DC_Pin, GPIO_PIN_SET); // Write data

    // CS Low
    HAL_GPIO_WritePin(LcdHandle->Init.CS_Port, LcdHandle->Init.CS_Pin, GPIO_PIN_RESET);

    uint16_t lineBuffer[240]; // Une ligne de pixels (240 pour ILI9341)
    for (int i = 0; i < 240; i++)
        lineBuffer[i] = (color >> 8) | (color << 8); // Convertir en Big Endian pour SPI

    for (int y = 0; y < 320; y++)
        HAL_SPI_Transmit(LcdHandle->Init.hspi, (uint8_t*) lineBuffer, sizeof(lineBuffer),
                HAL_MAX_DELAY);

    // CS High
    HAL_GPIO_WritePin(LcdHandle->Init.CS_Port, LcdHandle->Init.CS_Pin, GPIO_PIN_SET);
}

// Write character from font set to destination on screen
void ili9341_putchar(LCD_Handle *LcdHandle, int x, int y, int c, int size, int fcolor, int bcolor)
{
    int x0;
    int t0, t1, t2, t3, u;

    y = LcdHandle->height - y - FONTHEIGHT;

    x0 = x;
    for (t0 = 0; t0 < FONTWIDTH * 2; t0 += 2)
    {
        for (t1 = 0; t1 < size; t1++)
        {
            u = xchar[c][t0 + 1] + (xchar[c][t0] << 8);
            ili9341_set_xy(LcdHandle, x0, y);
            for (t2 = 16; t2 >= 0; t2--)
            {
                if (u & (1 << t2))
                {
                    for (t3 = 0; t3 < size; t3++)
                    {
                        ili9341_draw_pixel(LcdHandle, fcolor);
                    }
                }
                else
                {
                    for (t3 = 0; t3 < size; t3++)
                    {
                        ili9341_draw_pixel(LcdHandle, bcolor);
                    }
                }
            }
            x0++;
        }
    }
}

//Print String to LCD
void ili9341_putstring(LCD_Handle *LcdHandle, int x, int y, char *text, int size, int fc, int bc)
{
    int t1 = 0, x0;

    x0 = x;
    while (text[t1])
    {
        ili9341_putchar(LcdHandle, x0, y, text[t1], size, fc, bc);
        x0 += (size * FONTWIDTH);
        t1++;
    }
}


void ili9341_putnumber(LCD_Handle *LcdHandle, int x, int y, long num, int dec, int lsize, int fc, int bc)
{
    char *s = (char*) malloc(16);
    if (s != NULL)
    {
        int2asc(num, dec, s, 16);
        ili9341_putstring(LcdHandle, x, y, s, lsize, fc, bc);
        free(s);
    }
    else
    {
        ili9341_putstring(LcdHandle, x, y, (char*) "Error", lsize, fc, bc);
    }
}
