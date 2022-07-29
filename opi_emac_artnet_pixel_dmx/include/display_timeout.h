/**
 * @file display_timeout.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DISPLAY_TIMEOUT_H_
#define DISPLAY_TIMEOUT_H_

#include "h3_board.h"
#include "h3_gpio.h"

namespace display {
namespace timeout {

void gpio_init() {
	h3_gpio_fsel(KEY2_GPIO, GPIO_FSEL_INPUT);
	h3_gpio_pud(KEY2_GPIO, GPIO_PULL_UP);
}

bool gpio_renew() {
    return (h3_gpio_lev(KEY2_GPIO) == LOW);
}

}  // namespace timeout
}  // namespace display

#endif /* DISPLAY_TIMEOUT_H_ */
