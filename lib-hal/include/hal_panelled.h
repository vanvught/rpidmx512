/**
 * @file hal_panelled.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

/**
 * Must implement the following:
 *
 * namespace hal {
 * namespace panelled {
 * inline constexpr uint32_t ACTIVITY = ;
 * inline constexpr uint32_t ARTNET = ;
 * inline constexpr uint32_t DDP = ;
 * inline constexpr uint32_t SACN = ;
 * inline constexpr uint32_t LTC_IN = ;
 * inline constexpr uint32_t LTC_OUT = ;
 * inline constexpr uint32_t MIDI_IN = ;
 * inline constexpr uint32_t MIDI_OUT = ;
 * inline constexpr uint32_t OSC_IN = ;
 * inline constexpr uint32_t OSC_OUT = ;
 * inline constexpr uint32_t TCNET = ;
 *  DMX
 * inline constexpr uint32_t PORT_A_RX = ;
 * inline constexpr uint32_t PORT_A_TX = ;
 *
 * Assumption we can use (PORT_A_TX << nPortIndex)
 *
 * }  // namespace panelled
 *
 * void PanelLedOn(uint32_t)
 * void PanelLedOff(uint32_t)
 *
 * }  // namespace hal
 */

#ifndef HAL_PANELLED_H_
#define HAL_PANELLED_H_

#if defined(__linux__) || defined(__APPLE__)
#include "linux/hal_panelled.h"
#else
#if defined(H3)
#include "h3/hal_panelled.h"
#elif defined(GD32)
#include "gd32/hal_panelled.h"
#else
#include "rpi/hal_panelled.h"
#endif
#endif

#endif  // HAL_PANELLED_H_
