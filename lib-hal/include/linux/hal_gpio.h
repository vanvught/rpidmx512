/**
 * @file hal_gpio.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LINUX_HAL_GPIO_H_
#define LINUX_HAL_GPIO_H_

#if defined (RASPPI)
#define GPIO_PULL_UP	BCM2835_GPIO_PUD_UP
#else

#if !defined(LOW)
# define LOW	0
# define HIGH 	!LOW
#endif

enum GPIO_EXT {
	GPIO_EXT_7,
	GPIO_EXT_11,
	GPIO_EXT_12,
	GPIO_EXT_13,
	GPIO_EXT_15,
	GPIO_EXT_16,
	GPIO_EXT_18,
	GPIO_EXT_22,
	GPIO_EXT_29,
	GPIO_EXT_35,
	GPIO_EXT_26,
	GPIO_EXT_31,
	GPIO_EXT_36,
	GPIO_EXT_37,
	GPIO_EXT_38
};

enum GPIO_FSEL {
	GPIO_FSEL_OUTPUT, GPIO_FSEL_INPUT
};

enum GPIO_PULL {
	GPIO_PULL_UP, GPIO_PULL_DOWN, GPIO_PULL_DISABLE
};

# define FUNC_PREFIX(x) x
# ifdef __cplusplus
# include <cstdint>
extern "C" {
# else
# include <stdint.h>
# endif
  inline void GpioFsel([[maybe_unused]] uint8_t _p, [[maybe_unused]] uint8_t _q) { }
  inline void GpioSet([[maybe_unused]] uint8_t _p) { }
  inline void GpioClr([[maybe_unused]] uint8_t _p) { }
  inline uint8_t GpioLev([[maybe_unused]] uint8_t _p) { return 0; }
  inline void gpio_pud([[maybe_unused]] uint8_t _p, [[maybe_unused]] uint8_t _q) {}
# ifdef __cplusplus
}
# endif

#endif

#endif /* LINUX_HAL_GPIO_H_ */
