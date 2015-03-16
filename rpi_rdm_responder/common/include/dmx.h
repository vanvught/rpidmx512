/**
 * @file dmx.h
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

#ifndef DMX_H_
#define DMX_H_

#include <stdint.h>

#define DMX_DATA_BUFFER_SIZE	512 ///<

extern uint8_t dmx_data[DMX_DATA_BUFFER_SIZE];

typedef enum
{
	DMX_PORT_DIRECTION_IDLE = 0,	///<
	DMX_PORT_DIRECTION_OUTP = 1,	///< DMX output
	DMX_PORT_DIRECTION_INP = 2		///< DMX input
} _dmx_port_direction;

extern void dmx_init(void);
extern void dmx_port_direction_set(const uint8_t, const uint8_t);
extern uint8_t dmx_port_direction_get(void);
extern void dmx_data_send(const uint8_t *, const uint16_t);
extern void rdm_data_send(const uint8_t *, const uint16_t);
extern uint8_t rdm_available_get(void);
extern void rdm_available_set(uint8_t);
extern uint64_t rdm_data_receive_end_get(void);
extern uint8_t dmx_available_get(void);
extern void dmx_available_set(uint8_t);

#endif /* DMX_H_ */
