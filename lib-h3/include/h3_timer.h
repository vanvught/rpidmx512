/**
 * @file h_timer.c
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
#ifndef H3_TIMER_H_
#define H3_TIMER_H_

#define TIMER_CTRL_SINGLE_MODE	(1<<7)	///< When interval reached , the timer will disable automatically

// Timer IRQ Enable Register
#define TIMER_IRQ_EN_TMR0	(1 << 0)
#define TIMER_IRQ_EN_TMR1	(1 << 1)

// Timer IRQ Status Register
#define TIMER_IRQ_PEND_TMR0	(1 << 0)	///< Set will clear
#define TIMER_IRQ_PEND_TMR1	(1 << 1)	///< Set will clear

#ifdef __cplusplus
extern "C" {
#endif

extern void h3_timer_init(void);

extern void __msdelay(uint32_t);
extern void __usdelay(uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* H3_TIMER_H_ */
