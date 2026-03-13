/**
 * @file h3_platform_ltc.h
 *
 */
/* Copyright (C) 2022-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARM_H3_H3_PLATFORM_LTC_H_
#define ARM_H3_H3_PLATFORM_LTC_H_

#include "h3.h" // IWYU pragma: keep

/* ignore some GCC warnings */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "core_ca.h"

#pragma GCC diagnostic pop

#include "arm/arm.h"     // IWYU pragma: keep
#include "arm/gic.h"     // IWYU pragma: keep
#include "h3_timer.h"    // IWYU pragma: keep
#include "irq_timer.h"   // IWYU pragma: keep
#include "h3.h"          // IWYU pragma: keep
#include "h3_board.h"    // IWYU pragma: keep
#include "h3_gpio.h"     // IWYU pragma: keep
#include "h3_timer.h"    // IWYU pragma: keep
#include "h3_hs_timer.h" // IWYU pragma: keep
#include "irq_timer.h"   // IWYU pragma: keep

#endif // ARM_H3_H3_PLATFORM_LTC_H_
