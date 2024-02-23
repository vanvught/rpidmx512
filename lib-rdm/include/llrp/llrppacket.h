/**
 * @file llrppacket.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LLRPPACKET_H_
#define LLRPPACKET_H_

#include <cstdint>

#if  ! defined (PACKED)
#define PACKED __attribute__((packed))
#endif

namespace e133 {
static constexpr uint32_t LLRP_KNOWN_UID_SIZE = 200;
}  // namespace e133

/**
 * 5.4.1 General Format
 */
struct TRootLayerPreAmble {
	uint16_t PreAmbleSize;				///< Define RLP Preamble Size. Fixed 0x0010
	uint16_t PostAmbleSize;				///< RLP Post-amble Size. Fixed 0x0000
	uint8_t ACNPacketIdentifier[12];  	///< ACN Packet Identifier
}PACKED;

struct TRootLayerPDU {
	uint8_t FlagsLength[3]; 			///< Protocol flags and length. Low 20 bits = PDU length High 4 bits = 0xF
	uint32_t Vector;					///< Identifies RLP Data as LLRP Protocol PDU -> VECTOR_ROOT_LLRP
	uint8_t SenderCid[16];				///< Sender's CID. Sender's unique ID
}PACKED;

struct TLlrpPDU {
	uint8_t FlagsLength[3]; 			///< Protocol flags and length. Low 20 bits = PDU length High 4 bits = 0xF
	uint32_t Vector;					///< Identifies data format
	uint8_t DestinationCid[16];			///< The receiver's unique CID or the LLRP_BROADCAST_CID
	uint32_t TransactionNumber;			///< Used to match request / response messages.
}PACKED;

struct TProbeRequestPDU {
	uint8_t FlagsLength[3]; 			///< Protocol flags and length. Low 20 bits = PDU length High 4 bits = 0xF
	uint8_t Vector;						///< Identifies Identifies data as Probe Request -> VECTOR_PROBE_REQUEST_DATA
	uint8_t LowerUUID[6];				///< UID representing the lower UID bound of the Probe Request
	uint8_t UpperUUID[6];				///< UID representing the upper UID bound of the Probe Request
	uint16_t Filter;					///< Bitfield
	uint8_t KnownUUIDs[6 * e133::LLRP_KNOWN_UID_SIZE];///< Previously discovered UIDs
}PACKED;

struct TProbeReplyPDU {
	uint8_t FlagsLength[3]; 			///< Protocol flags and length. Low 20 bits = PDU length High 4 bits = 0xF
	uint8_t Vector;						///< Identifies Identifies data as Probe Reply -> VECTOR_PROBE_REPLY_DATA
	uint8_t UID[6];						///< The UID of the LLRP Target
	uint8_t HardwareAddress[6];			///< The numerically lowest hardware address of the host the LLRP Target is running on
	uint8_t ComponentType;				///< Indicates the type of Component responding. See Table A-23
}PACKED;

struct TRDMCommandPDU {
	uint8_t FlagsLength[3]; 			///< Protocol flags and length. Low 20 bits = PDU length High 4 bits = 0xF
	uint8_t Vector;						///< Identifies Identifies data as Probe Reply -> VECTOR_RDM_CMD_DATA
	uint8_t RDMData[232];				///< RDM packet excluding START Code
}PACKED;

struct TLLRPCommonPacket {
	struct TRootLayerPreAmble RootLayerPreAmble;
	struct TRootLayerPDU RootLayerPDU;
	struct TLlrpPDU LlrpPDU;
}PACKED;

struct TProbeRequestPDUPacket {
	struct TLLRPCommonPacket Common;
	struct TProbeRequestPDU ProbeRequestPDU;
}PACKED;

struct TTProbeReplyPDUPacket {
	struct TLLRPCommonPacket Common;
	struct TProbeReplyPDU ProbeReplyPDU;
}PACKED;

struct LTRDMCommandPDUPacket {
	struct TLLRPCommonPacket Common;
	struct TRDMCommandPDU RDMCommandPDU;
}PACKED;

#define LLRP_ROOT_LAYER_SIZE		sizeof(struct TRootLayerPDU)
#define LLRP_PDU_SIZE				sizeof(struct TLlrpPDU)
#define RDM_COMMAND_PDU_SIZE		sizeof(struct TRDMCommandPDU)

#define RDM_COMMAND_PDU_LENGTH(x)	static_cast<uint8_t>((RDM_COMMAND_PDU_SIZE - 232 + (x)))
#define RDM_LLRP_PDU_LENGHT(x)		static_cast<uint8_t>((LLRP_PDU_SIZE + RDM_COMMAND_PDU_LENGTH(x)))
#define RDM_ROOT_LAYER_LENGTH(x)	static_cast<uint8_t>((LLRP_ROOT_LAYER_SIZE + RDM_LLRP_PDU_LENGHT(x)))

#endif /* LLRPPACKET_H_ */
