#ifndef __NUM_KEYBOARD_DRIVER_H__
#define __NUM_KEYBOARD_DRIVER_H__



#include "stm32f4xx_hal.h"

#define NKB_NUM_KEYS 12

#define NKB_KEY_1 0x01
#define NKB_KEY_2 0x02
#define NKB_KEY_3 0x04
#define NKB_KEY_4 0x08
#define NKB_KEY_5 0x10
#define NKB_KEY_6 0x20
#define NKB_KEY_7 0x40
#define NKB_KEY_8 0x80
#define NKB_KEY_9 0x0100
#define NKB_KEY_ASTERISK 0x0200
#define NKB_KEY_0 0x0400
#define NKB_KEY_HASH 0x0800

#define NKB_DEBOUNCE_TICKS 3

typedef enum __NKB_IO
{
    NKB_ROW_IN_COL_OUT = 0,
    NKB_ROW_OUT_COL_IN = 1
}NKB_IO;

typedef struct __NKB_InitInfo
{
    /* Rows */
    uint32_t ROW1_Pin;
    GPIO_TypeDef *ROW1_Port;
    uint32_t ROW2_Pin;
    GPIO_TypeDef *ROW2_Port;
    uint32_t ROW3_Pin;
    GPIO_TypeDef *ROW3_Port;
    uint32_t ROW4_Pin;
    GPIO_TypeDef *ROW4_Port;

    /* Columns */
    uint32_t COLA_Pin;
    GPIO_TypeDef *COLA_Port;
    uint32_t COLB_Pin;
    GPIO_TypeDef *COLB_Port;
    uint32_t COLC_Pin;
    GPIO_TypeDef *COLC_Port;

    NKB_IO io;
} NKB_InitInfo;

typedef struct __NKB_Handle
{
    NKB_InitInfo Init;

    uint16_t PressedKeys;

    uint16_t LastPressed;

    uint16_t ConsumableKeyPressed;
    uint16_t ConsumableKeyReleased;

    uint16_t ConsumedKeyPressed;
    uint16_t ConsumedKeyReleased;

    uint8_t DebounceCounters[NKB_NUM_KEYS];
} NKB_Handle;

void NKB_Init(NKB_Handle* hnkb);
void NKB_Update(NKB_Handle* hnkb);
uint8_t NKB_IsKeyPressed(NKB_Handle* hnkb, uint16_t key);

uint8_t NKB_TryConsumeOnKeyPressed(NKB_Handle* hnkb, uint16_t key);
uint8_t NKB_TryConsumeOnKeyReleased(NKB_Handle* hnkb, uint16_t key);

#endif // __NUM_KEYBOARD_DRIVER_H__
