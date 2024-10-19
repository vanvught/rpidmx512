/**
 * @file l6470paramsconst.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef L6470PARAMSCONST_H_
#define L6470PARAMSCONST_H_

struct L6470ParamsConst {
	static inline const char MIN_SPEED[] = "l6470_min_speed";
	static inline const char MAX_SPEED[] = "l6470_max_speed";
	static inline const char ACC[] = "l6470_acc";
	static inline const char DEC[] = "l6470_dec";
	static inline const char KVAL_HOLD[] = "l6470_kval_hold";
	static inline const char KVAL_RUN[] = "l6470_kval_run";
	static inline const char KVAL_ACC[] = "l6470_kval_acc";
	static inline const char KVAL_DEC[] = "l6470_kval_dec";
	static inline const char MICRO_STEPS[] = "l6470_micro_steps";
};

#endif /* L6470PARAMSCONST_H_ */
