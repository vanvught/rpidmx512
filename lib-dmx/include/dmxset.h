/*
 * dmxset.h
 *
 *  Created on: 14 mei 2021
 *      Author: arjanvanvught
 */

#ifndef DMXSET_H_
#define DMXSET_H_

#include <cstdint>

namespace dmx {
enum class PortDirection {
	OUTP,
	INP
};
}  // namespace dmx

class DmxSet {
public:
	DmxSet();
	virtual ~DmxSet() {}

	virtual void SetPortDirection(uint32_t nPort, dmx::PortDirection tPortDirection, bool bEnableData)=0;

	virtual void RdmSendRaw(uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength)=0;

	virtual const uint8_t *RdmReceive(uint32_t nPort)=0;
	virtual const uint8_t *RdmReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut)=0;

	virtual uint32_t RdmGetDateReceivedEnd()=0;

	static DmxSet* Get() {
		return s_pThis;
	}

private:
	static DmxSet *s_pThis;
};

#endif /* IDMXSET_H_ */
