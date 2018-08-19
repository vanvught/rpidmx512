/**
 * @file h3_ccu.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef H3_CCU_H_
#define H3_CCU_H_

typedef enum H3_CCU_PLL {
	CCU_PLL_CPUX,
	CCU_PLL_AUDIO,
	CCU_PLL_VIDEO,
	CCU_PLL_VE,
	CCU_PLL_DDR,
	CCU_PLL_PERIPH0,
	CCU_PLL_GPU,
	CCU_PLL_PERIPH1,
	CCU_PLL_DE
} ccu_pll_t;

#define CCU_PERIPH0_CLOCK_HZ	600000000

#define CCU_PLL_CPUX_MIN_CLOCK_HZ	200000000	///< 200MHz
#define CCU_PLL_CPUX_MAX_CLOCK_HZ 	2600000000	///< 2.6GHz

#ifdef __cplusplus
extern "C" {
#endif

extern uint64_t h3_ccu_get_pll_rate(ccu_pll_t);

// NDEBUG
extern void h3_ccu_pll_dump(void);

#ifdef __cplusplus
}
#endif


#endif /* H3_CCU_H_ */
