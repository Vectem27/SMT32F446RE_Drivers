/*
 * project.h
 *
 *  Created on: Sep 27, 2025
 *      Author: Vectem
 */

#ifndef __PROJECT_H__
#define __PROJECT_H__

#include "stm32f4xx_hal.h"

void Init(void);

void Loop(uint32_t delta_ms);

void TimerInterupt(void);

#endif // __PROJECT_H__
