/**
 * @file common.h
 *
 */
/**
 * Structures and definitions from libartnet (c)Simon Newton and Lutz Hillebrand (ilLUTZminator), www.opendmx.net
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
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

#ifndef COMMON_H_
#define COMMON_H_

#define ARTNET_PROTOCOL_REVISION	14							///< Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016

#define NODE_ID						"Art-Net"					///< Array of 8 characters, the final character is a null termination. Value = A r t - N e t 0x00


/*
 * error codes
 */
enum {
	ARTNET_EOK = 0,
	ARTNET_ENET = -1,	///< network error
	ARTNET_EMEM = -2,	///< memory error
	ARTNET_EARG = -3,	///< argument error
	ARTNET_ESTATE = -4,	///< state error
	ARTNET_EACTION = -5	///< invalid action
};

/**
 * The maximum ports per node built into the ArtNet protocol.
 * This is always 4. Don't change it unless you really know what your doing
 */
enum {
	ARTNET_MAX_PORTS = 4
};

/**
 * The length of the short name field. Always 18
 */
enum {
	ARTNET_SHORT_NAME_LENGTH = 18
};

/**
 * The length of the long name field. Always 64
 */
enum {
	ARTNET_LONG_NAME_LENGTH = 64
};

/**
 * The length of the report field. Always 64
 */
enum {
	ARTNET_REPORT_LENGTH = 64
};

/**
 * The length of the DMX field. Always 512
 */
enum {
	ARTNET_DMX_LENGTH = 512
};

/**
 * Number of bytes in a RDM UID
 */
enum {
	ARTNET_RDM_UID_WIDTH = 6
};

/**
 * Length of the hardware address
 */
enum {
	ARTNET_MAC_SIZE = 6
};

/**
 * Length of the ESTA field
 */
enum {
	ARTNET_ESTA_SIZE = 2
};

/**
 * Length of the IP field
 */
enum {
	ARTNET_IP_SIZE = 4
};

/**
 * The Port is always 0x1936
 */
enum {
	ARTNET_UDP_PORT = 0x1936
};

#endif /* COMMON_H_ */
