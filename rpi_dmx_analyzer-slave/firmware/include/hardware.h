/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef HARDWARE_H_
#define HARDWARE_H_

extern void __disable_fiq(void);
extern void __enable_fiq(void);

extern void __disable_irq(void);
extern void __enable_irq(void);

extern void hardware_init(void);
extern void hardware_reboot(void);

extern const uint64_t hardware_uptime_seconds(void);
extern const uint32_t hardware_get_firmware_revision(void);
extern const uint8_t *hardware_get_firmware_copyright(void);
extern const uint8_t hardware_get_firmware_copyright_length(void);
extern const uint32_t hardware_get_board_model_id(void);
extern const uint8_t *hardware_get_board_model(void);
extern const uint8_t hardware_get_board_model_length(void);

#endif /* HARDWARE_H_ */
