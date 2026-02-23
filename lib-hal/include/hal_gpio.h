/**
 * @file hal_gpio.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef HAL_GPIO_H_
#define HAL_GPIO_H_

#if defined(__linux__) || defined(__APPLE__)
#include "linux/hal_gpio.h"
#include "linux/hal_api.h"
#elif defined(H3)
#include "h3/hal_gpio.h"
#include "h3/hal_api.h"
#elif defined(GD32)
#include "gd32/hal_gpio.h"
#include "gd32/hal_api.h"
#else
#include "rpi/hal_gpio.h"
#include "rpi/hal_api.h"
#endif

#if defined(RASPPI)
#define GPIO_EXT_7 RPI_V2_GPIO_P1_07
#define GPIO_EXT_11 RPI_V2_GPIO_P1_11
#define GPIO_EXT_12 RPI_V2_GPIO_P1_12
#define GPIO_EXT_13 RPI_V2_GPIO_P1_13
#define GPIO_EXT_15 RPI_V2_GPIO_P1_15
#define GPIO_EXT_16 RPI_V2_GPIO_P1_16
#define GPIO_EXT_18 RPI_V2_GPIO_P1_18
#define GPIO_EXT_22 RPI_V2_GPIO_P1_22
#define GPIO_EXT_24 RPI_V2_GPIO_P1_24
#define GPIO_EXT_26 RPI_V2_GPIO_P1_26
#define GPIO_EXT_29 RPI_V2_GPIO_P1_29
#define GPIO_EXT_31 RPI_V2_GPIO_P1_31
#define GPIO_EXT_32 RPI_V2_GPIO_P1_32
#define GPIO_EXT_33 RPI_V2_GPIO_P1_33
#define GPIO_EXT_35 RPI_V2_GPIO_P1_35
#define GPIO_EXT_36 RPI_V2_GPIO_P1_36
#define GPIO_EXT_37 RPI_V2_GPIO_P1_37
#define GPIO_EXT_38 RPI_V2_GPIO_P1_38
#define GPIO_EXT_40 RPI_V2_GPIO_P1_40

#define GPIO_FSEL_INPUT BCM2835_GPIO_FSEL_INPT
#define GPIO_FSEL_OUTPUT BCM2835_GPIO_FSEL_OUTP
#endif

#endif  // HAL_GPIO_H_
