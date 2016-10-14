/**
 * @file e131bridge.h
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

#ifndef E131BRIDGE_H_
#define E131BRIDGE_H_

#include <stdint.h>

#include "e131.h"
#include "lightset.h"

#include "sys_time.h"

#define MERGE_TIMEOUT_SECONDS		10							///<
#define PRIORITY_TIMEOUT_SECONDS	10							///<

/**
 *
 */
struct TE131BridgeState {
	uint8_t nPriority;
	bool IsNetworkDataLoss;
	bool IsMergeMode;				///< Is the Bridge in merging mode?
};

/**
 *
 */
struct TSource {
	uint32_t time;					///< The latest time of the data received from source
	uint32_t ip;					///< The IP address for source
	uint8_t data[E131_DMX_LENGTH];	///< The data received from source
	uint8_t cid[16];				///< Sender's CID. Sender's unique ID
	uint8_t sequenceNumber;
};

/**
 * struct to represent an output port
 *
 */
struct TOutputPort {
	uint8_t data[E131_DMX_LENGTH];	///< Data sent
	uint16_t length;				///< Length of sent DMX data
	TMerge mergeMode;				///< \ref TMerge
	struct TSource sourceA;
	struct TSource sourceB;
};

/**
 *
 */
class E131Bridge {
public:
	E131Bridge(void);
	~E131Bridge(void);

	void SetOutput(LightSet *);

	const uint8_t *GetSoftwareVersion(void);

	void Start(void);
	void Stop(void);

	const uint16_t getUniverse(void);
	void setUniverse(const uint16_t);

	int HandlePacket(void);

private:
	const bool IsValidPackage(void);
	void SetNetworkDataLossCondition(void);
	void CheckNetworkDataLoss(void);
	void CheckMergeTimeouts(void);
	const bool IsPriorityTimeOut(void);
	const bool isIpCidMatch(const struct TSource *);
	const bool IsDmxDataChanged(const uint8_t *, const uint16_t);
	const bool IsMergedDmxDataChanged(const uint8_t *, const uint16_t );
	void HandleDmx(void);

private:
	LightSet *m_pLightSet;
	uint16_t m_nUniverse;

	uint32_t m_nCurrentPacketMillis;
	uint32_t m_nPreviousPacketMillis;

	struct TE131BridgeState m_State;
	struct TOutputPort		m_OutputPort;

	struct TE131Packet m_E131Packet;
};

#endif /* E131BRIDGE_H_ */
