/**
 * @file dhcp_internal.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef DHCP_INTERNAL_H_
#define DHCP_INTERNAL_H_

enum DHCP_PORT {
	DHCP_PORT_SERVER = 67,
	DHCP_PORT_CLIENT = 68
};

enum DHCP_OP {
	DHCP_OP_BOOTREQUEST = 1,
	DHCP_OP_BOOTREPLY = 2
};

enum DHCP_HTYPE {
	DHCP_HTYPE_10MB = 1,
	DHCP_HTYPE_100MB = 2
};

enum DCHP_TYPE {
	DCHP_TYPE_DISCOVER = 1,
	DCHP_TYPE_OFFER = 2,
	DCHP_TYPE_REQUEST = 3,
	DCHP_TYPE_DECLINE = 4,
	DCHP_TYPE_ACK = 5,
	DCHP_TYPE_NAK = 6,
	DCHP_TYPE_RELEASE = 7,
	DCHP_TYPE_INFORM = 8
};

enum DHCP_STATE {
	DHCP_STATE_DHCP_INIT = 0,
	DHCP_STATE_DHCP_DISCOVER = 1,
	DHCP_STATE_DHCP_REQUEST = 2,
	DHCP_STATE_DHCP_LEASED = 3,
	DHCP_STATE_DHCP_REREQUEST = 4,
	DHCP_STATE_DHCP_RELEASE = 5,
	DHCP_STATE_DHCP_STOP = 6
};

#define DHCP_OPT_SIZE	312

#define MAGIC_COOKIE	0x63825363  ///< You should not modify it number.

#endif /* DHCP_INTERNAL_H_ */
