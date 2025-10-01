#ifndef __SNAKE_H__
#define __SNAKE_H__

#include "num_keyboard_driver.h"
#include "lcd_driver.h"

#define SNAKE_TILE_TYPE_EMPTY 0x00
#define SNAKE_TILE_TYPE_SNAKE 0x01
#define SNAKE_TILE_TYPE_FOOD 0x02
#define SNAKE_TILE_TYPE_WALL 0x03

#define SNAKE_TILE_X_COUNT 32
#define SNAKE_TILE_Y_COUNT 24
#define SNAKE_TILE_COUNT 768 // 32 * 24

typedef struct _SnakeTile
{
    uint8_t dirty :1;
    uint8_t type :3;
    uint16_t next_snake_tile :10;

    uint8_t reserved :3;
} SnakeTile;

typedef struct _SnakeInit
{
    LCD_Handle* lcd_handle;
    NKB_Handle* nkb_handle;
} SnakeInit;

typedef struct _SnakeGameState
{
    SnakeInit Init;

    uint16_t score;
    uint8_t is_running:1;
    uint8_t dir:2;

    uint16_t snake_head :10;
    uint16_t snake_tail :10;

    SnakeTile tiles[SNAKE_TILE_COUNT];
} SnakeGameState;

void InitSnake(SnakeGameState* gameState);

/*
 * return if the player lost
 */
uint8_t UpdateSnake(SnakeGameState* gameState);

#endif // __SNAKE_H__
