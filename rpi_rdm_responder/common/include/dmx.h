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
#include <stdbool.h>

#include "util.h"

#define DMX_DATA_BUFFER_SIZE					513		///< including SC
#define RDM_DATA_BUFFER_SIZE					512		///<
#define RDM_DATA_BUFFER_INDEX_SIZE 				0x0F	///<

#define DMX_TRANSMIT_BREAK_TIME_MIN				92		///< 92 us
#define DMX_TRANSMIT_BREAK_TIME_TYPICAL			176		///< 176 us
#define DMX_TRANSMIT_MAB_TIME_MIN				12		///< 12 us
#define DMX_TRANSMIT_MAB_TIME_MAX				1E6		///< 1s
#define DMX_TRANSMIT_REFRESH_DEFAULT			(uint32_t)(1E6 / 40)	///< 25000 us
#define DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN	1204	///< us

#define DMX_MIN_SLOT_VALUE 						0		///< The minimum value a DMX512 slot can take.
#define DMX_MAX_SLOT_VALUE 						255		///< The maximum value a DMX512 slot can take.
#define DMX512_START_CODE						0		///< The start code for DMX512 data. This is often referred to as NSC for "Null Start Code".

enum {
	DMX_UNIVERSE_SIZE = 512								///< The number of slots in a DMX512 universe.
};

typedef enum {
	DMX_PORT_DIRECTION_OUTP = 1,						///< DMX output
	DMX_PORT_DIRECTION_INP = 2							///< DMX input
} _dmx_port_direction;

struct _dmx_statistics {
	uint32_t mark_after_break;							///<
	uint32_t slots_in_packet;							///<
	uint32_t break_to_break;							///<
	uint32_t updates_per_seconde;						///<
	uint32_t slot_to_slot;								///<
};

struct _total_statistics {
	uint32_t dmx_packets;								///<
	uint32_t rdm_packets;								///<
};

extern uint8_t dmx_data[DMX_DATA_BUFFER_SIZE]; //TODO remove

extern void dmx_init(void);
extern void dmx_set_port_direction(const _dmx_port_direction, const bool);
extern const _dmx_port_direction dmx_get_port_direction(void);
extern void dmx_data_send(const uint8_t *, const uint16_t);
extern const bool dmx_get_available(void);
extern void dmx_set_available_false(void);
extern const uint32_t dmx_get_output_break_time(void);
extern void dmx_set_output_break_time(const uint32_t);
extern const uint32_t dmx_get_output_mab_time(void);
extern void dmx_set_output_mab_time(const uint32_t);
extern bool dmx_is_data_changed(void);
extern void dmx_reset_total_statistics(void);
extern /*@shared@*/const volatile struct _total_statistics *dmx_get_total_statistics(void) ASSUME_ALIGNED;
extern /*@shared@*/const volatile struct _dmx_statistics *dmx_get_statistics(void) ASSUME_ALIGNED;
extern const uint16_t dmx_get_send_data_length(void);
extern void dmx_set_send_data_length(uint16_t);
extern const uint32_t dmx_get_output_period(void);
extern void dmx_set_output_period(const uint32_t);
extern /*@shared@*/const /*@null@*/uint8_t *rdm_get_available(void) ASSUME_ALIGNED;
extern /*@shared@*/const uint8_t *rdm_get_current_data(void) ASSUME_ALIGNED;
extern void rdm_available_set(const uint8_t);
extern const uint32_t rdm_get_data_receive_end(void);

#endif /* DMX_H_ */
