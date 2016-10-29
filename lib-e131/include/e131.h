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

///< ANSI E1.31 — 2016 Entertainment Technology
///< Lightweight streaming protocol for transport of DMX512 using ACN


// TODO Update section references to 2016

/**
 * 3.2 Universe: A set of up to 512 data slots identified by universe number. 
 * Note: In E1.31 there may be multiple sources for a universe.
 */
enum {
	E131_DMX_LENGTH = 512
};

/**
 *
 */
enum TVectorRoot {
	E131_VECTOR_ROOT_DATA		= 0x00000004,	///< 
	E131_VECTOR_ROOT_EXTENDED	= 0x00000008	///<
};

/**
 * All E1.31 data packets shall carry the vector VECTOR_E131_DATA_PACKET in the Framing Layer
 * in order to indicate that their payload is DMX512-A data.
 */
enum TVectorData {
	E131_VECTOR_DATA_PACKET = 0x00000002		///< E1.31 Data Packet
};

/**
 *
 */
enum TVectorExtended {
	E131_VECTOR_EXTENDED_SYNCHRONIZATION	= 0x00000001,	///< E1.31 Synchronization Packet
	E131_VECTOR_EXTENDED_DISCOVERY			= 0x00000002	///< E1.31 Universe Discovery
};

/**
 *
 */
enum TVectorDMP {
	E131_VECTOR_DMP_SET_PROPERTY	= 0x02			///< (Informative)
};

#define VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST 0x00000001

/**
 * When multicast addressing is used, the UDP destination Port shall be set to the standard ACN-SDT
 * multicast port (5568).
 */
#define E131_DEFAULT_PORT		5568	///<

/**
 * Merge is implemented in either LTP or HTP mode
 */
enum TMerge {
	E131_MERGE_HTP,		///< Highest Takes Precedence (HTP)
	E131_MERGE_LTP		///< Latest Takes Precedence (LTP)
};

/**
 * 6.4 Priority
 *
 * No priority outside the range of 0 to 200 shall be transmitted on the network.
 * Priority increases with numerical value, i.e., 200 is a higher priority than 100.
 */
enum TPriority {
	E131_PRIORITY_LOWEST	= 1,	///<
	E131_PRIORITY_HIGHEST	= 200	///<
};

/**
 * 6.2.6 Options
 */
enum TOptions {
	E131_OPTIONS_MASK_PREVIEW_DATA 			= (1 << 7),	///< Preview Data: Bit 7 (most significant bit)
	E131_OPTIONS_MASK_STREAM_TERMINATED 	= (1 << 6),	///< Stream Terminated: Bit 6
	E131_OPTIONS_MASK_FORCE_SYNCHRONIZATION = (1 << 5)	///< Force Synchronization: Bit 5
};

/**
 * 6.7 Universe
 * The Universe is a 16-bit field that defines the universe number of the data carried in the packet. Universe
 * values shall be limited to the range 1 to 63999. Universe value 0 and those between 64000 and 65535 are
 * reserved for future use. See Section 8 for more information.
 */
/**
  * 9.1 Association of Multicast Addresses and Universe
 *
 * Universe numbers between 64000 and 65535, excluding universe 64214 (which is used for E1.31 Universe Discovery),
 * are reserved for future use and shall not be used by E1.31 devices except as specified in future standards
 * designed to coexist with E1.31 when use of these universes is called for specifically.
 */

enum TUniverse {
	E131_UNIVERSE_DEFAULT		= 1,		///<
	E131_UNIVERSE_MAX			= 63999,	///<
	E131_UNIVERSE_DISCOVERY		= 64214		///< Universe Discovery
};

#define E131_MERGE_TIMEOUT_SECONDS					10	///<
#define E131_PRIORITY_TIMEOUT_SECONDS				10	///<
#define E131_UNIVERSE_DISCOVERY_INTERVAL_SECONDS	10	///<
#define E131_NETWORK_DATA_LOSS_TIMEOUT_SECONDS		2.5	///<

#define E131_CID_LENGTH					16
#define E131_SOURCE_NAME_LENGTH			64
#define E131_PACKET_IDENTIFIER_LENGTH	12

#endif /* E131_H_ */
