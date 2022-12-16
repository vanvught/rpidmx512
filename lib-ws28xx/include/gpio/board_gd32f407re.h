/**
 * @file board_gd32f407re.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef GPIO_BOARD_GD32F407RE_H_
#define GPIO_BOARD_GD32F407RE_H_

#include "gd32.h"

#define RCU_GPIOx				RCU_GPIOC
#define GPIOx					GPIOC
#define GPIO_PINx				(GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13)
#define GPIO_PIN_OFFSET			6U

#define MASTER_TIMER_CLOCK		(APB2_CLOCK_FREQ * 2)

/**
 * Implementation note: CLOCK is Timer 2 Channel 0 is GPIOA6
 */

#define DEBUG_CS_RCU_GPIOx		RCU_GPIOA
#define DEBUG_CS_GPIOx			GPIOA
#define DEBUG_CS_GPIO_PINx		GPIO_PIN_14

#endif /* GPIO_BOARD_GD32F407RE_H_ */
