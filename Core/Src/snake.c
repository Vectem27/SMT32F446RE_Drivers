/*
 * snake.c
 *
 *  Created on: Oct 1, 2025
 *      Author: Vectem
 */
#include "snake.h"

#define TILE_SIZE 10

#define SNAKE_TILE(x, y) ((y) * SNAKE_TILE_X_COUNT + (x))
#define SNAKE_TILE_X(x) ((x) % SNAKE_TILE_X_COUNT)
#define SNAKE_TILE_Y(x) ((y) / SNAKE_TILE_X_COUNT)

SnakeTile* GetSnakeTile(SnakeGameState *gameState, uint8_t x, uint8_t y)
{
    return &(gameState->tiles[y * SNAKE_TILE_X_COUNT + x]);
}

void DrawSnakeTile(SnakeGameState *gameState, uint16_t tile)
{
    uint8_t x = tile % SNAKE_TILE_X_COUNT;
    uint8_t y = tile / SNAKE_TILE_X_COUNT;

    SnakeTile *snake_tile;
    snake_tile = GetSnakeTile(gameState, x, y);


    if (snake_tile->dirty == 0)
        return;

    for (int i = 0; i < TILE_SIZE; ++i)
    {
        for (int j = 0; j < TILE_SIZE; ++j)
        {

            gameState->Init.lcd_handle->SetDrawPos(gameState->Init.lcd_handle, x * TILE_SIZE + j, y * TILE_SIZE + i);

            uint16_t color = ((x & 0x1) == (y & 0x1)) ? LIGHTGREEN : GREEN;     // Faster than tile % 2 == 0

            if (snake_tile->type == SNAKE_TILE_TYPE_SNAKE)
            {
                if (gameState->snake_head == tile)
                    color = BLUE;
                else
                    color = LIGHTBLUE;
            }
            else if (snake_tile->type == SNAKE_TILE_TYPE_FOOD)
                color = RED;

            gameState->Init.lcd_handle->DrawPixel(gameState->Init.lcd_handle, color);
        }
    }

    snake_tile->dirty = 0;
}

void DrawSnakeToScreen(SnakeGameState *gameState)
{
    for (int i = 0; i < SNAKE_TILE_COUNT; ++i)
    {
        if (!gameState->tiles[i].dirty)
            continue;

        DrawSnakeTile(gameState, i);
    }
}

void InitSnake(SnakeGameState *gameState)
{
    for (int i = 0; i < SNAKE_TILE_COUNT; ++i)
    {
        gameState->tiles[i].dirty = 1;
        gameState->tiles[i].type = SNAKE_TILE_TYPE_EMPTY;
    }

    GetSnakeTile(gameState, SNAKE_TILE_X_COUNT * 0.5, SNAKE_TILE_Y_COUNT * 0.5)->type = SNAKE_TILE_TYPE_SNAKE;
    GetSnakeTile(gameState, SNAKE_TILE_X_COUNT * 0.5 + 1, SNAKE_TILE_Y_COUNT * 0.5)->type = SNAKE_TILE_TYPE_SNAKE;
    GetSnakeTile(gameState, SNAKE_TILE_X_COUNT * 0.5 + 1, SNAKE_TILE_Y_COUNT * 0.5)->next_snake_tile = SNAKE_TILE(
            SNAKE_TILE_X_COUNT * 0.5, SNAKE_TILE_Y_COUNT * 0.5);
    GetSnakeTile(gameState, SNAKE_TILE_X_COUNT * 0.5 + 2, SNAKE_TILE_Y_COUNT * 0.5)->type = SNAKE_TILE_TYPE_SNAKE;
    GetSnakeTile(gameState, SNAKE_TILE_X_COUNT * 0.5 + 2, SNAKE_TILE_Y_COUNT * 0.5)->next_snake_tile = SNAKE_TILE(
            SNAKE_TILE_X_COUNT * 0.5 + 1, SNAKE_TILE_Y_COUNT * 0.5);

    gameState->snake_head = SNAKE_TILE(SNAKE_TILE_X_COUNT * 0.5, SNAKE_TILE_Y_COUNT * 0.5);
    gameState->snake_tail = SNAKE_TILE(SNAKE_TILE_X_COUNT * 0.5 + 2, SNAKE_TILE_Y_COUNT * 0.5);

    GetSnakeTile(gameState, 4, 5)->type = SNAKE_TILE_TYPE_FOOD;
    GetSnakeTile(gameState, 12, 6)->type = SNAKE_TILE_TYPE_FOOD;
    GetSnakeTile(gameState, 21, 15)->type = SNAKE_TILE_TYPE_FOOD;

    DrawSnakeToScreen(gameState);

    gameState->dir = 0;
    gameState->is_running = 1;
    gameState->score = 0;
}

uint8_t UpdateSnake(SnakeGameState *gameState)
{
    if (!gameState->is_running)
        return 0;

    uint16_t newHeadIdx = gameState->snake_head;

    switch (gameState->dir)
    {
    case 0:
        newHeadIdx -= 1;
        break;
    case 1:
        newHeadIdx += SNAKE_TILE_X_COUNT;
        break;
    case 2:
        newHeadIdx += 1;
        break;
    case 3:
        newHeadIdx -= SNAKE_TILE_X_COUNT;
        break;
    }

    if (newHeadIdx < 0 || newHeadIdx >= SNAKE_TILE_COUNT || SNAKE_TILE_X(newHeadIdx) >= SNAKE_TILE_X_COUNT)
        newHeadIdx = gameState->snake_head;

    if (gameState->tiles[newHeadIdx].type == SNAKE_TILE_TYPE_WALL)
        newHeadIdx = gameState->snake_head;

    if (newHeadIdx == gameState->snake_head)
    {
        gameState->is_running = 0;
        return 1;
    }

    if (gameState->tiles[newHeadIdx].type == SNAKE_TILE_TYPE_FOOD)
    {
        gameState->score++;
        // TODO: Update the score display;
    }
    else
    {
        // Update tail if snake size didn't change
        gameState->tiles[gameState->snake_tail].dirty = 1;
        gameState->tiles[gameState->snake_tail].type = SNAKE_TILE_TYPE_EMPTY;

        gameState->snake_tail = gameState->tiles[gameState->snake_tail].next_snake_tile;
    }

    // Update head
    gameState->tiles[gameState->snake_head].dirty = 1;
    gameState->tiles[gameState->snake_head].next_snake_tile = newHeadIdx;

    gameState->snake_head = newHeadIdx;
    gameState->tiles[gameState->snake_head].dirty = 1;
    gameState->tiles[gameState->snake_head].type = SNAKE_TILE_TYPE_SNAKE;

    DrawSnakeToScreen(gameState);

    return 0;
}

