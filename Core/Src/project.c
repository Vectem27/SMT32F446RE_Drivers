/*
 * project.c
 *
 *  Created on: Sep 27, 2025
 *      Author: Vectem
 */
#include "project.h"

/* Standard includes */
#include <stdio.h>
#include <string.h>

/* Project includes */
#include "main.h"
#include "spi.h"

/* Driver includes */
#include "ili9341_driver.h"
#include "sd_spi_driver.h"

LCD_Handle hlcd;
SD_SPI_Handle hsd;

void Init(void)
{
    uint8_t row = 0;

    /* Lcd screen init */
    {
        hlcd.Init.hspi = &hspi1;
        hlcd.Init.CS_Pin = LCD_CS_Pin;
        hlcd.Init.CS_Port = LCD_CS_GPIO_Port;
        hlcd.Init.DC_Pin = LCD_DC_Pin;
        hlcd.Init.DC_Port = LCD_DC_GPIO_Port;
        hlcd.Init.RESET_Pin = LCD_RESET_Pin;
        hlcd.Init.RESET_Port = LCD_RESET_GPIO_Port;
        hlcd.Init.bg_color = BLACK;

        ili9341_init(&hlcd);

        hlcd.Clear(&hlcd);
        hlcd.PrintString(&hlcd, 0, 20 * row++, "Hello World!", 1, WHITE, hlcd.Init.bg_color);
    }

    /* Sd card init */
    {
        hsd.init.hspi = &hspi1;
        hsd.init.CS_Pin = SD_CS_Pin;
        hsd.init.CS_Port = SD_CS_GPIO_Port;

        SD_Error res = SD_Init(&hsd);
        if (res != SD_RESPONSE_NO_ERROR)
        {
            char str[32];
            sprintf(str, "Failed to init SD : 0x%x", res);

            hlcd.PrintString(&hlcd, 0, 20 * row++, str, 1, WHITE, hlcd.Init.bg_color);
        }


        uint8_t buffer[512];
        memset(buffer, 0x00, 512);
        res = SD_SectorRead(&hsd, 0x00, buffer);
        if (res != SD_RESPONSE_NO_ERROR)
        {
            char str[32];
            sprintf(str, "Failed to read SD : 0x%x", res);

            hlcd.PrintString(&hlcd, 0, 20 * row++, str, 1, WHITE, hlcd.Init.bg_color);
        }
        else
        {
            buffer[20] = '\0';
            hlcd.PrintString(&hlcd, 0, 20 * row++, buffer, 1, YELLOW, hlcd.Init.bg_color);
        }
    }

    hlcd.PrintString(&hlcd, 0, 20 * row++, "Init finished", 1, WHITE, hlcd.Init.bg_color);
}

static uint16_t FPS;

void Loop(uint32_t ticks)
{
    FPS = 60000 / ticks;
}

