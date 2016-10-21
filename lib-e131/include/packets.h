/**
 * @file packets.h
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

#ifndef PACKETS_H_
#define PACKETS_H_

#include <stdint.h>

#include "e131.h"

#if  ! defined (PACKED)
#define PACKED __attribute__((packed))
#endif

/**
 * Root Layer (See Section 5)
 */
struct TRootLayer {
	uint16_t PreAmbleSize;				///< Define RLP Preamble Size. Fixed 0x0010
	uint16_t PostAmbleSize;				///< RLP Post-amble Size. Fixed 0x0000
	uint8_t ACNPacketIdentifier[12];  	///< ACN Packet Identifier
	uint16_t FlagsLength; 				///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;					///< Identifies RLP Data as 1.31 Protocol PDU 0x00000004
	uint8_t Cid[E131_CID_LENGTH];		///< Sender's CID. Sender's unique ID
}PACKED;

/**
 * E1.31 Framing Layer (See Section 6)
 */
struct TDataFrameLayer {
	uint16_t FLagsLength;				///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;					///< Identifies 1.31 data as DMP Protocol PDU. Fixed 0x00000002
	uint8_t SourceName[64];				///< User Assigned Name of Source. UTF-8 [UTF-8] encoded string, null-terminated
	uint8_t Priority;					///< Data priority if multiple sources. 0-200, default of 100
	uint16_t Reserved;					///< Reserved. Transmitter Shall Send 0. Receivers Shall Ignore
	uint8_t SequenceNumber;				///< Sequence Number. To detect duplicate or out of order packets
	uint8_t Options;					///< Options Flags Bit. 7 = Preview_Data Bit 6 = Stream_Terminated
	uint16_t Universe;					///< Universe Number. Identifier for a distinct stream of DMX Data
}PACKED;

/**
 * DMP Layer (See Section 7)
 */
struct TDataDMPLayer {
	uint16_t FlagsLength;				///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint8_t Vector;						///< Identifies DMP Set Property Message PDU. Fixed 0x02
	uint8_t Type;						///< Identifies format of address and data. Fixed 0xa1
	uint16_t FirstAddressProperty;		///< Indicates DMX START Code is at DMP address 0. Fixed 0x0000
	uint16_t AddressIncrement;			///< Indicates each property is 1 octet 0x0001
	uint16_t PropertyValueCount;		///< Indicates 1+ the number of slots in packet. 0x0001 -- 0x0201
	uint8_t PropertyValues[513];		///< DMX512-A START Code + data. START Code + Data
}PACKED;

/**
 *
 */
struct TE131DataPacket {
	struct TDataFrameLayer FrameLayer;		///<
	struct TDataDMPLayer DMPLayer;			///< In DMP terms the DMX packet is treated at the DMP layer as a set property message for an array of up to 513 one-octet virtually addressed properties.
}PACKED;


/**
 *
 */
struct TDiscoveryFrameLayer {
	uint16_t FLagsLength;				///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;					///<
	uint8_t SourceName[64];				///< User Assigned Name of Source. UTF-8 [UTF-8] encoded string, null-terminated
	uint32_t Reserved;					///<
}PACKED;

/**
 *
 */
struct TUniverseDiscoveryLayer {
	uint16_t FlagsLength;				///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;					///<
	uint8_t Page;						///<
	uint8_t LastPage;					///<
	uint16_t ListOfUniverses[512];		///<
}PACKED;

/**
 *
 */
struct TE131DiscoveryPacket {
	struct TRootLayer RootLayer;
	struct TDiscoveryFrameLayer FrameLayer;
	struct TUniverseDiscoveryLayer UniverseDiscoveryLayer;
}PACKED;

/**
 *
 */
struct TRawFrameLayer {
	uint16_t FLagsLength;				///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;					///< Identifies 1.31 data as DMP Protocol PDU.
}PACKED;

/**
 * The E1.31 packet
 */
struct TE131 {
	struct TRootLayer RootLayer;		///< E1.31 shall use the ACN Root Layer Protocol as defined in the ANSI E1.17 [ACN] “ACN Architecture” document.
	union {
		struct TRawFrameLayer RawFrameLayer;
		struct TE131DataPacket DataPacket;
	};

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


#endif /* PACKETS_H_ */
