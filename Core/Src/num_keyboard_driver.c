/*
 * num_keyboard_driver.c
 *
 *  Created on: Sep 28, 2025
 *      Author: Vectem
 */

#include "num_keyboard_driver.h"

#define NKB_KEYS_MASK ((1U << NKB_NUM_KEYS) - 1)


uint8_t NKB_CheckRow(NKB_Handle *hnkb, uint8_t row)
{
    switch (row)
    {
    case 0:
        return HAL_GPIO_ReadPin(hnkb->Init.ROW1_Port, hnkb->Init.ROW1_Pin) == GPIO_PIN_SET;
    case 1:
        return HAL_GPIO_ReadPin(hnkb->Init.ROW2_Port, hnkb->Init.ROW2_Pin) == GPIO_PIN_SET;
    case 2:
        return HAL_GPIO_ReadPin(hnkb->Init.ROW3_Port, hnkb->Init.ROW3_Pin) == GPIO_PIN_SET;
    case 3:
        return HAL_GPIO_ReadPin(hnkb->Init.ROW4_Port, hnkb->Init.ROW4_Pin) == GPIO_PIN_SET;
    default:
        return 0;
    }
}

uint8_t NKB_CheckCol(NKB_Handle *hnkb, uint8_t col)
{
    switch (col)
    {
    case 0:
        return HAL_GPIO_ReadPin(hnkb->Init.COLA_Port, hnkb->Init.COLA_Pin) == GPIO_PIN_SET;
    case 1:
        return HAL_GPIO_ReadPin(hnkb->Init.COLB_Port, hnkb->Init.COLB_Pin) == GPIO_PIN_SET;
    case 2:
        return HAL_GPIO_ReadPin(hnkb->Init.COLC_Port, hnkb->Init.COLC_Pin) == GPIO_PIN_SET;
    default:
        return 0;
    }
}

void NKB_SetRow(NKB_Handle *hnkb, uint8_t row, GPIO_PinState pinState)
{
    switch (row)
    {
    case 0:
        HAL_GPIO_WritePin(hnkb->Init.ROW1_Port, hnkb->Init.ROW1_Pin, pinState);
        return;
    case 1:
        HAL_GPIO_WritePin(hnkb->Init.ROW2_Port, hnkb->Init.ROW2_Pin, pinState);
        return;
    case 2:
        HAL_GPIO_WritePin(hnkb->Init.ROW3_Port, hnkb->Init.ROW3_Pin, pinState);
        return;
    case 3:
        HAL_GPIO_WritePin(hnkb->Init.ROW4_Port, hnkb->Init.ROW4_Pin, pinState);
        return;
    }
}

void NKB_SetCol(NKB_Handle *hnkb, uint8_t col, GPIO_PinState pinState)
{
    switch (col)
    {
    case 0:
        HAL_GPIO_WritePin(hnkb->Init.COLA_Port, hnkb->Init.COLA_Pin, pinState);
        return;
    case 1:
        HAL_GPIO_WritePin(hnkb->Init.COLB_Port, hnkb->Init.COLB_Pin, pinState);
        return;
    case 2:
        HAL_GPIO_WritePin(hnkb->Init.COLC_Port, hnkb->Init.COLC_Pin, pinState);
        return;
    }
}

/*
 * row: 0-3
 * col: 0-2
 */
uint16_t NKB_GetKeyFromRowAndCol(uint8_t row, uint8_t col)
{
    uint8_t coord = row * 3 + col;

    switch (coord)
    {
    case 9:
        return NKB_KEY_ASTERISK;
    case 10:
        return NKB_KEY_0;
    case 11:
        return NKB_KEY_HASH;
    default:
        return 0x1 << coord;
    }
}

void NKB_InitRowPin(uint32_t Pin, GPIO_TypeDef *Port, NKB_IO io)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    GPIO_InitStruct.Pin = Pin;

    if (io == NKB_ROW_IN_COL_OUT)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }
    else
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    }
    HAL_GPIO_Init(Port, &GPIO_InitStruct);
}

void NKB_InitColPin(uint32_t Pin, GPIO_TypeDef *Port, NKB_IO io)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    GPIO_InitStruct.Pin = Pin;

    if (io == NKB_ROW_OUT_COL_IN)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }
    else
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    }
    HAL_GPIO_Init(Port, &GPIO_InitStruct);
}

void NKB_Init(NKB_Handle *hnkb)
{
    hnkb->PressedKeys = 0x00;

    NKB_InitColPin(hnkb->Init.COLA_Pin, hnkb->Init.COLA_Port, hnkb->Init.io);
    NKB_InitColPin(hnkb->Init.COLB_Pin, hnkb->Init.COLB_Port, hnkb->Init.io);
    NKB_InitColPin(hnkb->Init.COLC_Pin, hnkb->Init.COLC_Port, hnkb->Init.io);

    NKB_InitRowPin(hnkb->Init.ROW1_Pin, hnkb->Init.ROW1_Port, hnkb->Init.io);
    NKB_InitRowPin(hnkb->Init.ROW2_Pin, hnkb->Init.ROW2_Port, hnkb->Init.io);
    NKB_InitRowPin(hnkb->Init.ROW3_Pin, hnkb->Init.ROW3_Port, hnkb->Init.io);
    NKB_InitRowPin(hnkb->Init.ROW4_Pin, hnkb->Init.ROW4_Port, hnkb->Init.io);

    hnkb->ConsumableKeyPressed = 0x00;
    hnkb->ConsumableKeyReleased = 0x00;

    hnkb->ConsumedKeyPressed = 0xFFFF;
    hnkb->ConsumedKeyReleased = 0xFFFF;
}

void NKB_Update(NKB_Handle *hnkb)
{
    uint16_t rawKeys = 0x00;

    // --- ton scan inchangé, mais écrit dans rawKeys ---
    if (hnkb->Init.io == NKB_ROW_IN_COL_OUT)
    {
        for (int i = 0; i < 3; ++i)
        {
            NKB_SetCol(hnkb, i, GPIO_PIN_SET);

            for (int j = 0; j < 4; ++j)
            {
                if (NKB_CheckRow(hnkb, j))
                    rawKeys |= NKB_GetKeyFromRowAndCol(j, i);
            }

            NKB_SetCol(hnkb, i, GPIO_PIN_RESET);
        }
    }
    else if (hnkb->Init.io == NKB_ROW_OUT_COL_IN)
    {
        for (int i = 0; i < 4; ++i)
        {
            NKB_SetRow(hnkb, i, GPIO_PIN_SET);

            for (int j = 0; j < 3; ++j)
            {
                if (NKB_CheckCol(hnkb, j))
                    rawKeys |= NKB_GetKeyFromRowAndCol(i, j);
            }

            NKB_SetRow(hnkb, i, GPIO_PIN_RESET);
        }
    }
    // --- fin du scan brut ---

    // === étape anti-rebond ===
    uint16_t filtered = hnkb->PressedKeys; // base sur dernier état stable

    for (int k = 0; k < NKB_NUM_KEYS; ++k)
    {
        uint16_t mask = (1U << k);

        if (rawKeys & mask) {
            if (hnkb->DebounceCounters[k] < NKB_DEBOUNCE_TICKS)
                hnkb->DebounceCounters[k]++;

            if (hnkb->DebounceCounters[k] >= NKB_DEBOUNCE_TICKS)
                filtered |= mask; // validé comme pressé
        } else {
            if (hnkb->DebounceCounters[k] > 0)
                hnkb->DebounceCounters[k]--;

            if (hnkb->DebounceCounters[k] == 0)
                filtered &= ~mask; // validé comme relâché
        }
    }

    hnkb->PressedKeys = filtered;
    // === fin anti-rebond ===

    // --- transitions inchangées ---
    const uint16_t maskAll = NKB_KEYS_MASK;

    uint16_t prev = hnkb->LastPressed & maskAll;
    uint16_t now  = hnkb->PressedKeys & maskAll;

    uint16_t newlyPressed  = now & ~prev;
    uint16_t newlyReleased = (~now & maskAll) & prev;

    hnkb->ConsumedKeyPressed  &= ~newlyReleased;
    hnkb->ConsumedKeyReleased &= ~newlyPressed;

    hnkb->ConsumableKeyPressed  = now & ~hnkb->ConsumedKeyPressed;
    hnkb->ConsumableKeyReleased = (~now & maskAll) & ~hnkb->ConsumedKeyReleased;

    hnkb->LastPressed = now;
}



uint8_t NKB_IsKeyPressed(NKB_Handle *hnkb, uint16_t key)
{
    return (key & hnkb->PressedKeys) != 0x00;
}

// consomme les bits valides dans 'keyMask' qui sont consommables (press)
uint8_t NKB_TryConsumeOnKeyPressed(NKB_Handle *hnkb, uint16_t keyMask)
{
    const uint16_t valid = keyMask & hnkb->ConsumableKeyPressed;
    if (valid == 0) return 0;

    hnkb->ConsumedKeyPressed |= valid;
    hnkb->ConsumableKeyPressed &= ~valid;
    return 1;
}

// consomme les bits valides dans 'keyMask' qui sont consommables (release)
uint8_t NKB_TryConsumeOnKeyReleased(NKB_Handle *hnkb, uint16_t keyMask)
{
    const uint16_t valid = keyMask & hnkb->ConsumableKeyReleased;
    if (valid == 0) return 0;

    // optionnel : s'assurer que pressed est aussi marqué consommé
    // NKB_TryConsumeOnKeyPressed(hnkb, valid);

    hnkb->ConsumedKeyReleased |= valid;
    hnkb->ConsumableKeyReleased &= ~valid;

    // une release consommée permet à la prochaine press d'être à nouveau consommable
    hnkb->ConsumedKeyPressed &= ~valid;

    return 1;
}


