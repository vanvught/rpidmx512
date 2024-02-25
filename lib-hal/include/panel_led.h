/**
 * @file panel_led.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PANEL_LED_H_
#define PANEL_LED_H_

/**
 * Must implement the following:
 *
 * namespace hal {
 * namespace panelled {
 * static constexpr uint32_t ACTIVITY = ;
 * static constexpr uint32_t ARTNET = ;
 * static constexpr uint32_t DDP = ;
 * static constexpr uint32_t SACN = ;
 * static constexpr uint32_t LTC_IN = ;
 * static constexpr uint32_t LTC_OUT = ;
 * static constexpr uint32_t MIDI_IN = ;
 * static constexpr uint32_t MIDI_OUT = ;
 * static constexpr uint32_t OSC_IN = ;
 * static constexpr uint32_t OSC_OUT = ;
 * static constexpr uint32_t TCNET = ;
 *  DMX
 * static constexpr uint32_t PORT_A_RX = ;
 * static constexpr uint32_t PORT_A_TX = ;
 *
 * Assumption we can use (PORT_A_TX << nPortIndex)
 *
 * }  // namespace panelled
 *
 * void panel_led_on(uint32_t)
 * void panel_led_off(uint32_t)
 *
 * }  // namespace hal
*/

#if defined(__linux__) || defined (__APPLE__)
# include "linux/panel_led.h"
#else
# if defined (H3)
#  include "h3/panel_led.h"
# elif defined (GD32)
#  include "gd32/panel_led.h"
# else
#  include "rpi/panel_led.h"
# endif
#endif

#endif /* PANEL_LED_H_ */
