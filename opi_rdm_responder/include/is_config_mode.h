/**
 * @file is_config_mode.h
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef IS_CONFIG_MODE_H_
#define IS_CONFIG_MODE_H_

#include "h3_board.h"
#include "h3_gpio.h"

#include "debug.h"

void config_mode_init() {
	h3_gpio_fsel(KEY1_GPIO, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(KEY1_GPIO, GPIO_PULL_UP);
}

bool is_config_mode() {
	const auto nLevel = h3_gpio_lev(KEY1_GPIO);
    const auto isConfigMode = (nLevel == LOW);

    DEBUG_PRINTF("isConfigMode=%s %u %u->%p", isConfigMode ? "Yes" : "No", KEY1_GPIO, nLevel, H3_PIO_PORTA->DAT);

    return isConfigMode;
}

#endif /* IS_CONFIG_MODE_H_ */
