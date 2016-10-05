/**
 * @file e131.h
 *
 */

/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef E131_H_
#define E131_H_

#include <stdint.h>

#if  ! defined (PACKED)
#define PACKED __attribute__((packed))
#endif

#define E131_DEFAULT_PORT	5568	///<

/**
 * union of supported E1.31 packets
 */
struct TE131 {
	/* Root Layer */
	uint16_t PreAmbleSize;
	uint16_t PostAmbleSize;
	uint8_t acn_id[12];
	uint16_t root_flength;
	uint32_t root_vector;
	uint8_t cid[16];

	/* Frame Layer */
	uint16_t frame_flength;
	uint32_t frame_vector;
	uint8_t source_name[64];
	uint8_t priority;
	uint16_t reserved;
	uint8_t sequence_number;
	uint8_t options;
	uint16_t universe;

	/* DMP Layer */
	uint16_t dmp_flength;
	uint8_t dmp_vector;
	uint8_t type;
	uint16_t first_address;
	uint16_t address_increment;
	uint16_t property_value_count;
	uint8_t property_values[513];
}PACKED;

/**
 *
 */
struct TE131Packet {
	int length;				///<
	uint32_t IPAddressFrom;	///<
	uint32_t IPAddressTo;	///<
	struct TE131 E131;		///<
};


#endif /* E131_H_ */
