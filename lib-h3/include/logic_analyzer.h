/**
 * @file logic_analyzer.h
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LOGIC_ANALYZER_H_
#define LOGIC_ANALYZER_H_

#include "board/logic_analyzer.h"
#include "h3.h"

namespace logic_analyzer {
inline void init() {
#if defined (LOGIC_ANALYZER)
# if defined (LOGIC_ANALYZER_CH0_GPIO_PINx)
	h3_gpio_fsel(LOGIC_ANALYZER_CH0_GPIO_PINx, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(LOGIC_ANALYZER_CH0_GPIO_PINx);
# endif

# if defined (LOGIC_ANALYZER_CH1_GPIO_PINx)
	h3_gpio_fsel(LOGIC_ANALYZER_CH1_GPIO_PINx, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(LOGIC_ANALYZER_CH1_GPIO_PINx);
# endif

# if defined (LOGIC_ANALYZER_CH2_GPIO_PINx)
	h3_gpio_fsel(LOGIC_ANALYZER_CH2_GPIO_PINx, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(LOGIC_ANALYZER_CH2_GPIO_PINx);
# endif

# if defined (LOGIC_ANALYZER_CH3_GPIO_PINx)
	h3_gpio_fsel(LOGIC_ANALYZER_CH3_GPIO_PINx, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(LOGIC_ANALYZER_CH3_GPIO_PINx);
# endif

# if defined (LOGIC_ANALYZER_CH4_GPIO_PINx)
	h3_gpio_fsel(LOGIC_ANALYZER_CH4_GPIO_PINx, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(LOGIC_ANALYZER_CH4_GPIO_PINx);
# endif

# if defined (LOGIC_ANALYZER_CH5_GPIO_PINx)
	h3_gpio_fsel(LOGIC_ANALYZER_CH5_GPIO_PINx, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(LOGIC_ANALYZER_CH5_GPIO_PINx);
# endif

# if defined (LOGIC_ANALYZER_CH6_GPIO_PINx)
	h3_gpio_fsel(LOGIC_ANALYZER_CH6_GPIO_PINx, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(LOGIC_ANALYZER_CH6_GPIO_PINx);
# endif

# if defined (LOGIC_ANALYZER_CH7_GPIO_PINx)
	h3_gpio_fsel(LOGIC_ANALYZER_CH7_GPIO_PINx, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(LOGIC_ANALYZER_CH7_GPIO_PINx);
# endif

#endif
}

inline void ch0_clear() {
#if defined (LOGIC_ANALYZER_CH0_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_clr(LOGIC_ANALYZER_CH0_GPIO_PINx);
#endif
}

inline void ch0_set() {
#if defined (LOGIC_ANALYZER_CH0_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_set(LOGIC_ANALYZER_CH0_GPIO_PINx);
#endif
}

inline void ch1_clear() {
#if defined (LOGIC_ANALYZER_CH1_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_clr(LOGIC_ANALYZER_CH1_GPIO_PINx);
#endif
}

inline void ch1_set() {
#if defined (LOGIC_ANALYZER_CH1_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_set(LOGIC_ANALYZER_CH1_GPIO_PINx);
#endif
}

inline void ch2_clear() {
#if defined (LOGIC_ANALYZER_CH2_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_clr(LOGIC_ANALYZER_CH2_GPIO_PINx);
#endif
}

inline void ch2_set() {
#if defined (LOGIC_ANALYZER_CH2_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_set(LOGIC_ANALYZER_CH2_GPIO_PINx);
#endif
}

inline void ch3_clear() {
#if defined (LOGIC_ANALYZER_CH3_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_clr(LOGIC_ANALYZER_CH3_GPIO_PINx);
#endif
}

inline void ch3_set() {
#if defined (LOGIC_ANALYZER_CH3_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_set(LOGIC_ANALYZER_CH3_GPIO_PINx);
#endif
}

inline void ch4_clear() {
#if defined (LOGIC_ANALYZER_CH4_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_clr(LOGIC_ANALYZER_CH4_GPIO_PINx);
#endif
}

inline void ch4_set() {
#if defined (LOGIC_ANALYZER_CH4_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_set(LOGIC_ANALYZER_CH4_GPIO_PINx);
#endif
}

inline void ch5_clear() {
#if defined (LOGIC_ANALYZER_CH5_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_clr(LOGIC_ANALYZER_CH5_GPIO_PINx);
#endif
}

inline void ch5_set() {
#if defined (LOGIC_ANALYZER_CH5_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_set(LOGIC_ANALYZER_CH5_GPIO_PINx);
#endif
}

inline void ch6_clear() {
#if defined (LOGIC_ANALYZER_CH6_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_clr(LOGIC_ANALYZER_CH6_GPIO_PINx);
#endif
}

inline void ch6_set() {
#if defined (LOGIC_ANALYZER_CH6_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_set(LOGIC_ANALYZER_CH6_GPIO_PINx);
#endif
}

inline void ch7_clear() {
#if defined (LOGIC_ANALYZER_CH7_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_clr(LOGIC_ANALYZER_CH7_GPIO_PINx);
#endif
}

inline void ch7_set() {
#if defined (LOGIC_ANALYZER_CH7_GPIO_PINx) && defined (LOGIC_ANALYZER)
	h3_gpio_set(LOGIC_ANALYZER_CH7_GPIO_PINx);
#endif
}
}  // namespace logic_analyzer

#endif /* LOGIC_ANALYZER_H_ */
