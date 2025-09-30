/*
 * project.c
 *
 *  Created on: Sep 27, 2025
 *      Author: Vectem
 */
#include "project.h"

/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

/* Project includes */
#include "main.h"
#include "spi.h"

/* Driver includes */
#include "ili9341_driver.h"
#include "sd_spi_driver.h"
#include "num_keyboard_driver.h"

LCD_Handle hlcd;
SD_SPI_Handle hsd;
NKB_Handle hnkb;

uint8_t MAX_LINE_CHAR;
uint8_t MAX_ROW_CHAR;

uint32_t mem_counter = 0x4000;

void Init(void)
{

    uint16_t row = 0;

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

        MAX_LINE_CHAR = hlcd.width / FONTWIDTH;
        MAX_ROW_CHAR = hlcd.height / FONTHEIGHT;
    }

    /* Sd card init */
    {
        hsd.init.hspi = &hspi2;
        hsd.init.CS_Pin = SD_CS_Pin;
        hsd.init.CS_Port = SD_CS_GPIO_Port;

        SD_Error res = SD_Init(&hsd);
        if (res != SD_RESPONSE_NO_ERROR)
        {
            char str[32];
            sprintf(str, "Failed to init SD : 0x%x", res);

            hlcd.PrintString(&hlcd, 0, 20 * row++, str, 1, WHITE, hlcd.Init.bg_color);
        }

        /*SD_CSD sd_test_csd;
         SD_Bus_Hold(&hsd);
         SD_GetCSDRegister(&hsd, &sd_test_csd);
         SD_Bus_Release(&hsd);

         if (sd_test_csd.PermWrProtect)
         hlcd.PrintString(&hlcd, 0, 20 * row++, "SD is write perm protected", 1, WHITE, hlcd.Init.bg_color);

         if (sd_test_csd.TempWrProtect)
         hlcd.PrintString(&hlcd, 0, 20 * row++, "SD is write temp protected", 1, WHITE, hlcd.Init.bg_color);*/
    }

    /* Num keyboard init */
    {
        hnkb.Init.COLA_Pin = NKB_IN_COL_A_Pin;
        hnkb.Init.COLA_Port = NKB_IN_COL_A_GPIO_Port;
        hnkb.Init.COLB_Pin = NKB_IN_COL_B_Pin;
        hnkb.Init.COLB_Port = NKB_IN_COL_B_GPIO_Port;
        hnkb.Init.COLC_Pin = NKB_IN_COL_C_Pin;
        hnkb.Init.COLC_Port = NKB_IN_COL_C_GPIO_Port;

        hnkb.Init.ROW1_Pin = NKB_OUT_ROW_1_Pin;
        hnkb.Init.ROW1_Port = NKB_OUT_ROW_1_GPIO_Port;
        hnkb.Init.ROW2_Pin = NKB_OUT_ROW_2_Pin;
        hnkb.Init.ROW2_Port = NKB_OUT_ROW_2_GPIO_Port;
        hnkb.Init.ROW3_Pin = NKB_OUT_ROW_3_Pin;
        hnkb.Init.ROW3_Port = NKB_OUT_ROW_3_GPIO_Port;
        hnkb.Init.ROW4_Pin = NKB_OUT_ROW_4_Pin;
        hnkb.Init.ROW4_Port = NKB_OUT_ROW_4_GPIO_Port;

        hnkb.Init.io = NKB_ROW_IN_COL_OUT;

        NKB_Init(&hnkb);
    }

    /* Sd card check */
    {
        HAL_Delay(250);
        uint8_t res;

        /* Write */
        for (int i = 0; i < 2; ++i)
        {
            uint8_t tx_buffer[512];
            memset(tx_buffer, '\0', 512);
            if (i == 0)
                sprintf((char*) tx_buffer, "Hello world (Write 1) !");
            else
                sprintf((char*) tx_buffer, "Hello world (Write 2) !");

            res = SD_SectorWrite(&hsd, 0x50, tx_buffer);
            if (res != SD_RESPONSE_NO_ERROR)
            {
                char str[22];
                sprintf(str, "Failed to write SD : 0x%x", res);
                hlcd.PrintString(&hlcd, 0, 20 * row++, str, 1, WHITE, hlcd.Init.bg_color);
            }

            /* Read */
            uint8_t rx_buffer[512];
            memset(rx_buffer, '0', 512);
            res = SD_SectorRead(&hsd, 0x50, rx_buffer);
            if (res != SD_RESPONSE_NO_ERROR)
            {
                char str[22];
                sprintf(str, "Failed to read SD : 0x%x", res);
                hlcd.PrintString(&hlcd, 0, 20 * row++, str, 1, WHITE, hlcd.Init.bg_color);
            }
            else
            {
                rx_buffer[26] = '\0';
                hlcd.PrintString(&hlcd, 0, 20 * row++, (char*) rx_buffer, 1, YELLOW, hlcd.Init.bg_color);
            }
        }
    }

    //hlcd.Clear(&hlcd);
    hlcd.PrintString(&hlcd, 0, ROW12, "Init finished", 1, WHITE, hlcd.Init.bg_color);
    //MemTest(0x400);
}

static uint16_t FPS;
uint8_t bDoOnce = 1;

struct KeyChar
{
    uint16_t key;
    char c;
};

struct KeyChar keyChars[NKB_NUM_KEYS] = { { NKB_KEY_1, '1' }, { NKB_KEY_2, '2' }, { NKB_KEY_3, '3' },
        { NKB_KEY_4, '4' }, { NKB_KEY_5, '5' }, { NKB_KEY_6, '6' }, { NKB_KEY_7, '7' }, { NKB_KEY_8, '8' }, { NKB_KEY_9,
                '9' }, { NKB_KEY_ASTERISK, '*' }, { NKB_KEY_0, '0' }, { NKB_KEY_HASH, '#' } };

uint16_t seek = 0;

void PrintAtSeek(char c)
{
    uint16_t x = 2 + FONTWIDTH * (seek % MAX_LINE_CHAR);
    uint16_t y = 2 + (FONTHEIGHT + 1) * (seek / MAX_LINE_CHAR);
    hlcd.PrintChar(&hlcd, x, y, c, 1, WHITE, hlcd.Init.bg_color);

    seek += 1;

    if (seek >= MAX_LINE_CHAR * MAX_ROW_CHAR)
    {
        seek = 0;
        hlcd.Clear(&hlcd);
    }

}

static volatile uint32_t tick_flag = 0;

uint32_t tim_count = 0;

void Loop(uint32_t ticks)
{
    //FPS = 60000 / ticks;

    NKB_Update(&hnkb);

    if (tick_flag >= 100)
    {

        tim_count += tick_flag;
        tick_flag = 0;
        hlcd.PrintString(&hlcd, 0, ROW5, "Timer interrupt", 1, MAROON1, hlcd.Init.bg_color);
        hlcd.PrintNumber(&hlcd, 0, ROW6, tim_count, 0, 1, MAROON2, hlcd.Init.bg_color);
    }

    /*for (int i = 0; i < NKB_NUM_KEYS; ++i)
     {
     if (NKB_TryConsumeOnKeyPressed(&hnkb, keyChars[i].key))
     {
     if (i == 9)
     {
     if (seek > 0)
     {
     --seek;
     PrintAtSeek(' ');
     --seek;
     }
     }
     else if (i == 11)
     {
     seek = 0;
     hlcd.Clear(&hlcd);
     }
     else
     PrintAtSeek(keyChars[i].c);
     }
     }*/
}

void TimerInterupt(void)
{
    tick_flag++;
}
