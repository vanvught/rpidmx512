/**
 * @file e131packets.h
 *
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef E131PACKETS_H_
#define E131PACKETS_H_

#include <cstdint>

#include "e131.h"
#include "e117.h"

#if  ! defined (PACKED)
# define PACKED __attribute__((packed))
#endif

/**
 * Root Layer (See Section 5)
 */
struct TRootLayer {
	uint16_t PreAmbleSize;						///< Define RLP Preamble Size. Fixed 0x0010
	uint16_t PostAmbleSize;						///< RLP Post-amble Size. Fixed 0x0000
	uint8_t ACNPacketIdentifier[12];  			///< ACN Packet Identifier
	uint16_t FlagsLength; 						///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;							///< Identifies RLP Data as 1.31 Protocol PDU 0x00000004
	uint8_t Cid[e131::CID_LENGTH];				///< Sender's CID. Sender's unique ID
}PACKED;

///< E1.31 Framing Layer (See Section 6)
struct TDataFrameLayer {
	uint16_t FLagsLength;						///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;							///< Identifies 1.31 data as DMP Protocol PDU. Fixed 0x00000002
	uint8_t SourceName[e131::SOURCE_NAME_LENGTH];///< User Assigned Name of Source. UTF-8 [UTF-8] encoded string, null-terminated
	uint8_t Priority;							///< Data priority if multiple sources. 0-200, default of 100
	uint16_t SynchronizationAddress;			///< Universe on which synchronization packets are transmitted
	uint8_t SequenceNumber;						///< Sequence Number. To detect duplicate or out of order packets
	uint8_t Options;							///< Options Flags Bit. 7 = Preview_Data Bit 6 = Stream_Terminated
	uint16_t Universe;							///< Universe Number. Identifier for a distinct stream of DMX Data
}PACKED;

///< DMP Layer (See Section 7)
///< In DMP terms the DMX packet is treated at the DMP layer as a set property message for an array of up to 513 one-octet virtually addressed properties.
struct TDataDMPLayer {
	uint16_t FlagsLength;						///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint8_t Vector;								///< Identifies DMP Set Property Message PDU. Fixed 0x02
	uint8_t Type;								///< Identifies format of address and data. Fixed 0xa1
	uint16_t FirstAddressProperty;				///< Indicates DMX START Code is at DMP address 0. Fixed 0x0000
	uint16_t AddressIncrement;					///< Indicates each property is 1 octet 0x0001
	uint16_t PropertyValueCount;				///< Indicates 1+ the number of slots in packet. 0x0001 -- 0x0201
	uint8_t PropertyValues[e131::DMX_LENGTH + 1];///< DMX512-A START Code + data. START Code + Data
}PACKED;

/**
 *
 */
struct TE131DataPacket {
	struct TRootLayer RootLayer;				///< E1.31 shall use the ACN Root Layer Protocol as defined in the ANSI E1.17 [ACN] “ACN Architecture” document.
	struct TDataFrameLayer FrameLayer;
	struct TDataDMPLayer DMPLayer;
}PACKED;

/**
 * 6.4 E1.31 Universe Discovery Packet Framing Layer
 */
struct TDiscoveryFrameLayer {
	uint16_t FLagsLength;						///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;							///< Identifies 1.31 data as Universe Discovery Data. \ref E131_VECTOR_EXTENDED_DISCOVERY
	uint8_t SourceName[e131::SOURCE_NAME_LENGTH];///< User Assigned Name of Source. UTF-8 [UTF-8] encoded string, null-terminated
	uint32_t Reserved;							///< Reserved
}PACKED;

/**
 * 8 Universe Discovery Layer
 */
struct TUniverseDiscoveryLayer {
	uint16_t FlagsLength;						///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;							///< Identifies Universe Discovery data as universe list. \ref VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST
	uint8_t Page;								///< Packet Number. Identifier indicating which packet of N this is—pages start numbering at 0.
	uint8_t LastPage;							///< Final Page. Page number of the final page to be transmitted.
	uint16_t ListOfUniverses[512];				///< Sorted list of up to 512 16-bit universes. Universes upon which data is being transmitted.
}PACKED;

/**
 *
 */
struct TE131DiscoveryPacket {
	struct TRootLayer RootLayer;				///< E1.31 shall use the ACN Root Layer Protocol as defined in the ANSI E1.17 [ACN] “ACN Architecture” document.
	struct TDiscoveryFrameLayer FrameLayer;
	struct TUniverseDiscoveryLayer UniverseDiscoveryLayer;
}PACKED;

/**
 * 6.3 E1.31 Synchronization Packet Framing Layer
 */
struct TE131SynchronizationFrameLayer {
	uint16_t FLagsLength;						///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;							///< Identifies 1.31 data as DMP Protocol PDU. Fixed 0x00000002
	uint8_t SequenceNumber;						///< Sequence Number. To detect duplicate or out of order packets
	uint16_t UniverseNumber;					///< Universe on which synchronization packets
	uint16_t Reserved;							///< Reserved. (See Section 6.3.4)
}PACKED;

/**
 * 4.2 E1.31 Synchronization Packet
 */
struct TE131SynchronizationPacket {
	struct TRootLayer RootLayer;				///< E1.31 shall use the ACN Root Layer Protocol as defined in the ANSI E1.17 [ACN] “ACN Architecture” document.
	struct TE131SynchronizationFrameLayer FrameLayer;
}PACKED;

/**
 *
 */
struct TRawFrameLayer {
	uint16_t FLagsLength;						///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
	uint32_t Vector;							///< Identifies 1.31 data as DMP Protocol PDU.
}PACKED;

/**
 *
 */
struct TE131RawPacket {
	struct TRootLayer RootLayer;				///< E1.31 shall use the ACN Root Layer Protocol as defined in the ANSI E1.17 [ACN] “ACN Architecture” document.
	struct TRawFrameLayer FrameLayer;
}PACKED;

/**
 * The E1.31 packet
 */
union UE131Packet {
	struct TE131RawPacket Raw;
	struct TE131DataPacket Data;
	struct TE131DiscoveryPacket Discovery;
	struct TE131SynchronizationPacket Synchronization;
};

#define ROOT_LAYER_SIZE						sizeof(struct TRootLayer)

#define DATA_FRAME_LAYER_SIZE				sizeof(struct TDataFrameLayer)
#define DATA_LAYER_SIZE						sizeof(struct TDataDMPLayer)

#define DISCOVERY_FRAME_LAYER_SIZE			sizeof(struct TDiscoveryFrameLayer)
#define DISCOVERY_LAYER_SIZE				sizeof(struct TUniverseDiscoveryLayer)

#define SYNCHRONIZATION_FRAME_LAYER_SIZE	sizeof(struct TE131SynchronizationFrameLayer)

#define DISCOVERY_LAYER_LENGTH(x)			(DISCOVERY_LAYER_SIZE - ((512U - (x)) * 2U))
#define DISCOVERY_FRAME_LAYER_LENGTH(x)		(DISCOVERY_FRAME_LAYER_SIZE + DISCOVERY_LAYER_LENGTH(x))
#define DISCOVERY_ROOT_LAYER_LENGTH(x)		(ROOT_LAYER_SIZE - 16U + DISCOVERY_FRAME_LAYER_LENGTH(x))

#define DISCOVERY_PACKET_SIZE(x)			(ROOT_LAYER_SIZE + DISCOVERY_FRAME_LAYER_SIZE + DISCOVERY_LAYER_LENGTH(x))

#define SYNCHRONIZATION_LAYER_LENGTH		(SYNCHRONIZATION_FRAME_LAYER_SIZE)
#define SYNCHRONIZATION_ROOT_LAYER_LENGTH	(ROOT_LAYER_SIZE - 16U + SYNCHRONIZATION_LAYER_LENGTH)

#define SYNCHRONIZATION_PACKET_SIZE			(ROOT_LAYER_SIZE + SYNCHRONIZATION_FRAME_LAYER_SIZE)

#define DATA_LAYER_LENGTH(x)				(DATA_LAYER_SIZE - 513U + (x))
#define DATA_FRAME_LAYER_LENGTH(x)			(DATA_FRAME_LAYER_SIZE + DATA_LAYER_LENGTH(x))
#define DATA_ROOT_LAYER_LENGTH(x)			(ROOT_LAYER_SIZE - 16U + DATA_FRAME_LAYER_LENGTH(x))

#define DATA_PACKET_SIZE(x)					(ROOT_LAYER_SIZE + DATA_FRAME_LAYER_SIZE + DATA_LAYER_LENGTH(x))

#endif /* E131PACKETS_H_ */
