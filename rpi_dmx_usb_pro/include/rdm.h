/**
 * @file rdm.h
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

#ifndef RDM_H_
#define RDM_H_

#include <stdint.h>

/**
 * The size of a UID.
 */
enum
{
	UID_SIZE = 6 /**< The size of a UID in binary form */
};

struct _rdm_command
{
	uint8_t start_code;						///< 1
	uint8_t sub_start_code;					///< 2
	uint8_t message_length;					///< 3
	uint8_t destination_uid[UID_SIZE];		///< 4,5,6,7,8,9
	uint8_t source_uid[UID_SIZE];			///< 10,11,12,13,14,15
	uint8_t transaction_number;				///< 16
	uint8_t port_id;						///< 17
	uint8_t message_count;					///< 18
	uint8_t sub_device[2];					///< 19, 20
	uint8_t command_class;					///< 21
	uint8_t param_id[2];					///< 22, 23
	uint8_t param_data_length;				///< 24
	uint8_t param_data[300];				///< 25,,,,
} ;

struct _rdm_discovery_msg {
	uint8_t header_FE[7];
	uint8_t header_AA;
	uint8_t masked_device_id[12];
	uint8_t checksum[4];
} ;

///< http://rdm.openlighting.org/pid/display?manufacturer=0&pid=96
struct _rdm_device_info
{
	uint8_t protocol_major;
	uint8_t protocol_minor;
	uint8_t device_model[2];
	uint8_t product_category[2];
	uint8_t software_version[4];
	uint8_t dmx_footprint[2];
	uint8_t current_personality;
	uint8_t personality_count;
	uint8_t dmx_start_address[2];
	uint8_t sub_device_count[2];
	uint8_t sensor_count;
};

// personalities
struct _rdm_personality
{
	uint16_t slots;
	const char *description;
};

#define RDM_DATA_BUFFER_SIZE			512 ///<
#define RDM_MESSAGE_MINIMUM_SIZE		24	///< Minimum size of RDM message without the checksum
#define RDM_MESSAGE_CHECKSUM_SIZE		2	///< Size of the checksum

#endif /* RDM_H_ */
