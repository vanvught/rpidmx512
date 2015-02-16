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
	uint8_t start_code;
	uint8_t sub_start_code;
	uint8_t message_length;
	uint8_t destination_uid[UID_SIZE];
	uint8_t source_uid[UID_SIZE];
	uint8_t transaction_number;
	uint8_t port_id;
	uint8_t message_count;
	uint16_t sub_device;
	uint8_t command_class;
	uint16_t param_id;
	uint8_t param_data_length;
	uint8_t param_data[60-24];
} ;


#endif /* RDM_H_ */
