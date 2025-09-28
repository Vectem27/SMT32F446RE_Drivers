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

void MemTest(uint32_t Addr);

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

        MAX_LINE_CHAR = hlcd.width / FONTWIDTH;
        MAX_ROW_CHAR = hlcd.height / FONTHEIGHT;
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

        SD_SCR sd_test_scr;
        SD_GetSCRRegister(&hsd, &sd_test_scr);

        if (sd_test_scr.Security != 0)
            hlcd.PrintString(&hlcd, 0, row++, "SD might be protected\n", 1, WHITE, hlcd.Init.bg_color);

        SD_CSD sd_test_csd;
        SD_GetCSDRegister(&hsd, &sd_test_csd);

        if (sd_test_csd.PermWrProtect)
            hlcd.PrintString(&hlcd, 0, row++, "SD is write perm protected", 1, WHITE, hlcd.Init.bg_color);

        if (sd_test_csd.TempWrProtect)
                    hlcd.PrintString(&hlcd, 0, row++, "SD is write temp protected", 1, WHITE, hlcd.Init.bg_color);
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

    hlcd.PrintString(&hlcd, 0, ROW12, "Init finished", 1, WHITE, hlcd.Init.bg_color);
    //hlcd.Clear(&hlcd);

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

void Loop(uint32_t ticks)
{
    //FPS = 60000 / ticks;

    NKB_Update(&hnkb);

    if (NKB_TryConsumeOnKeyPressed(&hnkb, NKB_KEY_0))
    {
        hlcd.PrintString(&hlcd, 0, 0, "                            ", 1, WHITE, hlcd.Init.bg_color);
        hlcd.PrintString(&hlcd, 0, 20, "                            ", 1, WHITE, hlcd.Init.bg_color);
        mem_counter += 0x4000;
        MemTest(mem_counter);
    }

    if (NKB_TryConsumeOnKeyPressed(&hnkb, NKB_KEY_HASH))
    {
        mem_counter = 256;
    }

    if (NKB_TryConsumeOnKeyPressed(&hnkb, NKB_KEY_ASTERISK))
    {
        mem_counter++;
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

void MemTest(uint32_t Addr)
{
    uint8_t row = 0;
    SD_Error res;

    static char str[32];

    static uint8_t buffer[512];

    static uint8_t new_buff[20];

    memset(buffer, 0x09, 512);
    res = SD_SectorWrite(&hsd, Addr, buffer);
    if (res != SD_RESPONSE_NO_ERROR)
    {
        sprintf(str, "Failed to write SD : 0x%x", res);
        hlcd.PrintString(&hlcd, 0, 20 * row++, str, 1, WHITE, hlcd.Init.bg_color);
    }

    memset(buffer, 0, 512);
    res = SD_SectorRead(&hsd, Addr, buffer);
    if (res != SD_RESPONSE_NO_ERROR)
    {

        sprintf(str, "Failed to read SD : 0x%x", res);
        hlcd.PrintString(&hlcd, 0, 20 * row++, str, 1, WHITE, hlcd.Init.bg_color);
    }
    else
    {
        int cur = 0;
        memset(new_buff, ' ', 20);
        for (int i = 0; i < 512; ++i)
        {
            if (buffer[i] != 0)
            {
                new_buff[cur] = buffer[i];
                if (++cur == 20)
                    break;
            }

        }
        hlcd.PrintString(&hlcd, 0, 20 * row++, (char*) new_buff, 1, YELLOW, hlcd.Init.bg_color);
        sprintf(str, "Block : 0x%02x", Addr);
        hlcd.PrintString(&hlcd, 0, 20 * row++, (char*) str, 1, WHITE, hlcd.Init.bg_color);
    }
}
