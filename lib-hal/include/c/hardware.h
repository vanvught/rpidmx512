/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef C_HARDWARE_H_
#define C_HARDWARE_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void hardware_reboot(void);
extern void hardware_led_init(void);
extern void hardware_led_set(const int);

extern uint32_t hardware_uptime_seconds(void);

extern int32_t hardware_firmware_get_revision(void);
extern const char *hardware_firmware_get_copyright(void);
extern uint8_t hardware_firmware_get_copyright_length(void);

extern int32_t hardware_board_get_model_id(void);
extern const char *hardware_board_get_model(void);
extern uint8_t hardware_board_get_model_length(void);
extern const char *hardware_board_get_soc(void);

extern int32_t hardware_get_core_temperature(void);

extern void hardware_rtc_set(const struct tm *);
extern bool hardware_rtc_get(struct tm *);

#ifdef __cplusplus
}
#endif

#ifdef H3
 #include "c/h3/hardware.h"
#else
 #include "c/rpi/hardware.h"
#endif

#endif /* C_HARDWARE_H_ */
