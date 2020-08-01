/**
 * @file ptp.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PTP_H_
#define PTP_H_

#include <stdint.h>

namespace ptp {
namespace udp {
static constexpr char MULTICAST_ADDRESS[] = "224.0.1.129";
namespace port {
static constexpr uint16_t EVENT = 319;
static constexpr uint16_t GENERAL = 320;
}  // namespace port
}  // namespace udp

enum MessageType: uint8_t {
	SYNC = 0x0,
	DELAY_REQ = 0x1,
	PDELAY_REQ = 0x2,
	PDELAY_RESP = 0x3,
	FOLLOW_UP = 0x8,
	DELAY_RESP = 0x9,
	PDELAY_RESP_FOLLOW_UP = 0xA,
	ANNOUNCE = 0xB,
	SIGNALING = 0xC,
	MANAGEMENT = 0xD
};
enum Flags: uint16_t {
	TWO_STEP = (1U << 9)
};

}  // namespace ptp

#if !defined (PACKED)
# define PACKED __attribute__((packed))
#endif

struct PTPHeader {
	uint8_t TransportType;
	uint8_t Version;
	uint16_t MessageLength;
	uint8_t DomainNumber;
	uint8_t Reserved1;
	uint16_t Flags;
	uint64_t CorrectionField;
	uint32_t Reserverd2;
	uint8_t ClockIdentity[8];
	uint8_t SourcePortIdentity[2];
	uint8_t SequenceId[2];
	uint8_t ControlField;
	uint8_t LogMessageInterval;
} PACKED;

struct PTPSync {
	struct PTPHeader Header;
	uint8_t Seconds[6];
	uint8_t NanoSeconds[4];
} PACKED;

struct PTPFollowUp {
	struct PTPHeader Header;
	uint8_t Epoch[2];
	uint8_t Seconds[4];
	uint8_t NanoSeconds[4];
} PACKED;

struct PTPDelayReq {
	struct PTPHeader Header;
	uint8_t Epoch[2];
	uint8_t Seconds[4];
	uint8_t NanoSeconds[4];
} PACKED;

struct PTPDelayResp {
	struct PTPHeader Header;
	uint8_t Epoch[2];
	uint8_t Seconds[4];
	uint8_t NanoSeconds[4];
	uint8_t RequestingSourcePortIdentity[6];
	uint8_t RequestingSourcePortId[2];
} PACKED;


struct PTPMessage {
	union {
		struct PTPHeader Header;
		struct PTPSync Sync;
		struct PTPFollowUp FollowUp;
		struct PTPDelayReq DelayReq;
		struct PTPDelayResp DelayResp;
	};
};

struct PTP {

};

#endif /* PTP_H_ */
