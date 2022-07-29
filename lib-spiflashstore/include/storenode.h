/**
 * @file storenode.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STORENODE_H_
#define STORENODE_H_

#include <stdint.h>
#include <cassert>

#include "nodeparams.h"
#include "nodestore.h"

#include "spiflashstore.h"

#include "debug.h"

class StoreNode final: public NodeParamsStore, public NodeStore {
public:
	StoreNode(uint32_t nPortIndexOffset = 0);

	void Update(const struct nodeparams::Params* pArtNetParams) override {
		DEBUG_ENTRY

		SpiFlashStore::Get()->Update(spiflashstore::Store::NODE, pArtNetParams, sizeof(struct nodeparams::Params));

		DEBUG_EXIT
	}

	void Copy(struct nodeparams::Params* pArtNetParams) override {
		DEBUG_ENTRY

		SpiFlashStore::Get()->Copy(spiflashstore::Store::NODE, pArtNetParams, sizeof(struct nodeparams::Params));

		DEBUG_EXIT
	}

	// Art-Net Handler -> ArtNetStore

	void SaveFailSafe(uint8_t nFailSafe) override {
		DEBUG_ENTRY

		SpiFlashStore::Get()->Update(spiflashstore::Store::NODE, __builtin_offsetof(struct nodeparams::Params, nFailSafe), &nFailSafe, sizeof(uint8_t), nodeparams::Mask::FAILSAFE);

		DEBUG_EXIT
	}

	void SaveShortName(const char *pShortName) override {
		DEBUG_ENTRY

		SpiFlashStore::Get()->Update(spiflashstore::Store::NODE, __builtin_offsetof(struct nodeparams::Params, aShortName), pShortName, artnet::SHORT_NAME_LENGTH, nodeparams::Mask::SHORT_NAME);

		DEBUG_EXIT
	}

	void SaveLongName(const char *pLongName) override {
		DEBUG_ENTRY

		SpiFlashStore::Get()->Update(spiflashstore::Store::NODE, __builtin_offsetof(struct nodeparams::Params, aLongName), pLongName, artnet::LONG_NAME_LENGTH, nodeparams::Mask::LONG_NAME);

		DEBUG_EXIT
	}

	void SaveMergeMode(uint32_t nPortIndex, lightset::MergeMode mergeMode) override {
		DEBUG_ENTRY
		DEBUG_PRINTF("%u, %u", nPortIndex, static_cast<uint32_t>(mergeMode));

		if (nPortIndex >= s_nPortIndexOffset) {
			nPortIndex -= s_nPortIndexOffset;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= nodeparams::MAX_PORTS) {
			DEBUG_EXIT
			return;
		}

		uint16_t nMergeMode;
		SpiFlashStore::Get()->Copy(spiflashstore::Store::NODE, &nMergeMode, sizeof(uint16_t), __builtin_offsetof(struct nodeparams::Params, nMergeMode), false);

		nMergeMode &= nodeparams::clear_mask(nPortIndex);

		if (mergeMode == lightset::MergeMode::LTP) {
			nMergeMode |= nodeparams::shift_left(static_cast<uint32_t>(lightset::MergeMode::LTP), nPortIndex);
			nMergeMode |= static_cast<uint16_t>(1U << (nPortIndex + 8));
		} else {
			nMergeMode |= nodeparams::shift_left(static_cast<uint32_t>(lightset::MergeMode::HTP), nPortIndex);
		}

		SpiFlashStore::Get()->Update(spiflashstore::Store::NODE, __builtin_offsetof(struct nodeparams::Params, nMergeMode), &nMergeMode, sizeof(uint16_t));

		DEBUG_EXIT
	}

	void SavePortProtocol(uint32_t nPortIndex, artnet::PortProtocol portProtocol) override {
		DEBUG_ENTRY
		DEBUG_PRINTF("%u, %u", nPortIndex, static_cast<uint32_t>(portProtocol));

		if (nPortIndex >= s_nPortIndexOffset) {
			nPortIndex -= s_nPortIndexOffset;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= nodeparams::MAX_PORTS) {
			DEBUG_EXIT
			return;
		}

		uint16_t nProtocol;
		SpiFlashStore::Get()->Copy(spiflashstore::Store::NODE, &nProtocol, sizeof(uint16_t), __builtin_offsetof(struct nodeparams::Params, nProtocol), false);

		nProtocol &= nodeparams::clear_mask(nPortIndex);

		if (portProtocol == artnet::PortProtocol::SACN) {
			nProtocol |= nodeparams::shift_left(static_cast<uint32_t>(node::PortProtocol::SACN), nPortIndex);
			nProtocol |= static_cast<uint16_t>(1U << (nPortIndex + 8));
		} else {
			nProtocol |= nodeparams::shift_left(static_cast<uint32_t>(node::PortProtocol::ARTNET), nPortIndex);
		}

		SpiFlashStore::Get()->Update(spiflashstore::Store::NODE, __builtin_offsetof(struct nodeparams::Params, nProtocol), &nProtocol, sizeof(uint16_t));

		DEBUG_EXIT
	}

	void SaveRdmEnabled(uint32_t nPortIndex, bool isEnabled) override {
		DEBUG_ENTRY
		DEBUG_PRINTF("nPortIndex=%u, isEnabled=%d", nPortIndex, isEnabled);

		if (nPortIndex >= s_nPortIndexOffset) {
			nPortIndex -= s_nPortIndexOffset;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= nodeparams::MAX_PORTS) {
			DEBUG_EXIT
			return;
		}

		uint16_t nRdm;
		SpiFlashStore::Get()->Copy(spiflashstore::Store::NODE, &nRdm, sizeof(uint16_t), __builtin_offsetof(struct nodeparams::Params, nRdm), false);

		nRdm &= nodeparams::clear_mask(nPortIndex);

		if (isEnabled) {
			nRdm |= nodeparams::shift_left(1, nPortIndex);
			nRdm |= static_cast<uint16_t>(1U << (nPortIndex + 8));
		}

		SpiFlashStore::Get()->Update(spiflashstore::Store::NODE, __builtin_offsetof(struct nodeparams::Params, nRdm), &nRdm, sizeof(uint16_t));

		DEBUG_EXIT
	}

	void SaveUniverse(uint32_t nPortIndex, uint16_t nUniverse) override {
		DEBUG_ENTRY
		DEBUG_PRINTF("nPortIndex=%u, nUniverse=%u", nPortIndex, nUniverse);

		if (nPortIndex >= s_nPortIndexOffset) {
			nPortIndex -= s_nPortIndexOffset;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= nodeparams::MAX_PORTS) {
			DEBUG_EXIT
			return;
		}

		SpiFlashStore::Get()->Update(spiflashstore::Store::NODE, (sizeof(uint16_t) * nPortIndex) + __builtin_offsetof(struct nodeparams::Params, nUniverse), &nUniverse, sizeof(uint16_t), nodeparams::Mask::UNIVERSE_A << nPortIndex);

		DEBUG_EXIT
	}

	void SaveUniverseSwitch(uint32_t nPortIndex, __attribute__((unused)) uint8_t nAddress) override {
		DEBUG_ENTRY
		DEBUG_PRINTF("nPortIndex=%u, nAddress=%u", nPortIndex, static_cast<uint32_t>(nAddress));

		if (nPortIndex >= s_nPortIndexOffset) {
			nPortIndex -= s_nPortIndexOffset;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= nodeparams::MAX_PORTS) {
			DEBUG_EXIT
			return;
		}

		uint16_t nPortAddress;
		ArtNetNode::Get()->GetPortAddress(nPortIndex, nPortAddress);
		SaveUniverse(nPortIndex, nPortAddress);

		DEBUG_EXIT
	}

	void SaveNetSwitch(uint32_t nPage, __attribute__((unused)) uint8_t nAddress) override {
		DEBUG_ENTRY
		DEBUG_PRINTF("nPage=%u, nAddress=%u", nPage, static_cast<uint32_t>(nAddress));

		if (nPage >= artnetnode::PAGES) {
			DEBUG_EXIT
			return;
		}

		if (artnetnode::PAGE_SIZE == 1) {
			uint16_t nPortAddress;
			ArtNetNode::Get()->GetPortAddress(nPage, nPortAddress);
			SaveUniverse(nPage, nPortAddress);
		}

		if (artnetnode::PAGE_SIZE == 4) {
			const auto nPortIndexStart = nPage * 4;
			for (uint32_t nPortIndex = nPortIndexStart; nPortIndex < (nPortIndexStart + 4); nPortIndex++) {
				uint16_t nPortAddress;
				ArtNetNode::Get()->GetPortAddress(nPortIndex, nPortAddress);
				SaveUniverse(nPortIndex, nPortAddress);
			}
		}

		DEBUG_EXIT
	}

	void SaveSubnetSwitch(uint32_t nPage, __attribute__((unused)) uint8_t nAddress) override {
		DEBUG_ENTRY
		DEBUG_PRINTF("nPage=%u, nAddress=%u", nPage, static_cast<uint32_t>(nAddress));

		if (nPage >= artnetnode::PAGES) {
			DEBUG_EXIT
			return;
		}

		if (artnetnode::PAGE_SIZE == 1) {
			uint16_t nPortAddress;
			ArtNetNode::Get()->GetPortAddress(nPage, nPortAddress);
			SaveUniverse(nPage, nPortAddress);
		}

		if (artnetnode::PAGE_SIZE == 4) {
			const auto nPortIndexStart = nPage * 4;
			for (uint32_t nPortIndex = nPortIndexStart; nPortIndex < (nPortIndexStart + 4); nPortIndex++) {
				uint16_t nPortAddress;
				ArtNetNode::Get()->GetPortAddress(nPortIndex, nPortAddress);
				SaveUniverse(nPortIndex, nPortAddress);
			}
		}

		DEBUG_EXIT
	}

	static StoreNode *Get() {
		return s_pThis;
	}

private:
	static uint32_t s_nPortIndexOffset;
	static StoreNode *s_pThis;
};

#endif /* STORENODE_H_ */
