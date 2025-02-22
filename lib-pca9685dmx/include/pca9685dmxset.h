/*
 * pca9685dmxset.h
 */

#ifndef PCA9685DMXSET_H_
#define PCA9685DMXSET_H_

#include <cstdint>

#include "dmxnode.h"

class PCA9685DmxSet {
public:
	PCA9685DmxSet() {}
	virtual ~PCA9685DmxSet() {}

	virtual void Start(const uint32_t nPortIndex)= 0;
	virtual void Stop(const uint32_t nPortIndex)= 0;
	virtual void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true)= 0;

	virtual void Sync(const uint32_t PortIndex)= 0;
	virtual void Sync()= 0;

	virtual bool SetDmxStartAddress(const uint16_t nDmxStartAddress)= 0;
	virtual uint16_t GetDmxStartAddress()= 0;
	virtual uint16_t GetDmxFootprint()= 0;
	virtual bool GetSlotInfo([[maybe_unused]] const uint16_t nSlotOffset, dmxnode::SlotInfo &slotInfo) {
		slotInfo.nType = 0x00; // ST_PRIMARY
		slotInfo.nCategory = 0x0001; // SD_INTENSITY
		return true;
	}

	virtual void Print() = 0;

	uint32_t GetUserData() { return 0; }		///< Art-Net ArtPollReply
	uint32_t GetRefreshRate() { return 0; }		///< Art-Net ArtPollReply

	void Blackout([[maybe_unused]] bool bBlackout) {}
	void FullOn() {}
};

#endif /* PCA9685DMXSET_H_ */
