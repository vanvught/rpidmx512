/**
 * @file artnetipprog.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef ARTNETIPPROG_H_
#define ARTNETIPPROG_H_

#include <stdint.h>

#if defined (__circle__)
#else
#include <stdbool.h>
#endif

#if  !defined (PACKED)
#define PACKED __attribute__((packed))
#endif

#define IPPROG_COMMAND_NONE					0
#define IPPROG_COMMAND_ENABLE_PROGRAMMING	(1<<7)
#define IPPROG_COMMAND_ENABLE_DHCP			((1<<6) | IPPROG_COMMAND_ENABLE_PROGRAMMING)
#define IPPROG_COMMAND_SET_TO_DEFAULT		((1<<3) | IPPROG_COMMAND_ENABLE_PROGRAMMING)
#define IPPROG_COMMAND_PROGRAM_IPADDRESS	((1<<2) | IPPROG_COMMAND_ENABLE_PROGRAMMING)
#define IPPROG_COMMAND_PROGRAM_SUBNETMASK	((1<<1) | IPPROG_COMMAND_ENABLE_PROGRAMMING)
#define IPPROG_COMMAND_PROGRAM_PORT			(0      | IPPROG_COMMAND_ENABLE_PROGRAMMING)

struct TArtNetIpProg {
	uint8_t Command;	///< Defines the how this packet is processed.
	uint8_t Filler;		///< Set to zero. Pads data structure for word alignment.
	uint8_t ProgIpHi;	///< IP Address to be programmed into Node if enabled by Command Field
	uint8_t ProgIp2;
	uint8_t ProgIp1;
	uint8_t ProgIpLo;
	uint8_t ProgSmHi;	///< Subnet mask to be programmed into Node if enabled by Command Field
	uint8_t ProgSm2;
	uint8_t ProgSm1;
	uint8_t ProgSmLo;
	uint8_t ProgPortHi;	///< PortAddress to be programmed into Node if enabled by Command Field
	uint8_t ProgPortLo;
} PACKED;

struct TArtNetIpProgReply {
	uint8_t ProgIpHi;	///< IP Address of Node
	uint8_t ProgIp2;
	uint8_t ProgIp1;
	uint8_t ProgIpLo;
	uint8_t ProgSmHi;	///< Subnet mask of Node
	uint8_t ProgSm2;
	uint8_t ProgSm1;
	uint8_t ProgSmLo;
	uint8_t ProgPortHi;	///< Port Address of Node
	uint8_t ProgPortLo;
	uint8_t Status;		///< Bit 6 DHCP enabled.
} PACKED;

class ArtNetIpProg {
public:
	virtual ~ArtNetIpProg(void);

	virtual void Handler(const struct TArtNetIpProg *, struct TArtNetIpProgReply *)= 0;

private:
};

#endif /* ARTNETIPPROG_H_ */
