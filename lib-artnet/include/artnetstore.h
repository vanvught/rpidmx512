/**
 * @file artnetstore.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETSTORE_H_
#define ARTNETSTORE_H_

#include <cstdint>
#include <cstddef>

#include "artnetparams.h"
#include "configstore.h"
#include "artnetnode.h"

#include "debug.h"

namespace artnetnode {
namespace configstore {
extern uint32_t DMXPORT_OFFSET;
}  // namespace configstore
}  // namespace artnetnode

class ArtNetStore {
public:
	static void SaveLongName(const char *pLongName) {
		DEBUG_ENTRY
		ConfigStore::Get()->Update(configstore::Store::NODE, offsetof(struct artnetparams::Params, aLongName), pLongName, artnet::LONG_NAME_LENGTH, artnetparams::Mask::LONG_NAME);
		DEBUG_EXIT

	}

	static void SaveShortName(uint32_t nPortIndex, const char *pShortName) {
		DEBUG_ENTRY
		DEBUG_PRINTF("%u, %s", nPortIndex, pShortName);

		if (nPortIndex >= artnetnode::configstore::DMXPORT_OFFSET) {
			nPortIndex -= artnetnode::configstore::DMXPORT_OFFSET;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= artnet::PORTS) {
			DEBUG_EXIT
			return;
		}

		ConfigStore::Get()->Update(configstore::Store::NODE, (artnet::SHORT_NAME_LENGTH * nPortIndex) + offsetof(struct artnetparams::Params, aLabel), pShortName, artnet::SHORT_NAME_LENGTH, artnetparams::Mask::LABEL_A << nPortIndex);

		DEBUG_EXIT
	}

	static void SaveUniverseSwitch(const uint32_t nPortIndex, [[maybe_unused]] const uint8_t nUniverseSwitch) {
		DEBUG_ENTRY
		SaveUniverse(nPortIndex);
		DEBUG_EXIT
	}

	static void SaveNetSwitch(const uint32_t nPortIndex, [[maybe_unused]] const uint8_t nNetSwitch) {
		DEBUG_ENTRY
		SaveUniverse(nPortIndex);
		DEBUG_EXIT
	}

	static void SaveSubnetSwitch(const uint32_t nPortIndex, [[maybe_unused]] const uint8_t nSubnetSwitch) {
		DEBUG_ENTRY
		SaveUniverse(nPortIndex);
		DEBUG_EXIT
	}

	static void SaveMergeMode(uint32_t nPortIndex, const lightset::MergeMode mergeMode) {
		DEBUG_ENTRY
		DEBUG_PRINTF("%u, %u", nPortIndex, static_cast<uint32_t>(mergeMode));

		if (nPortIndex >= artnetnode::configstore::DMXPORT_OFFSET) {
			nPortIndex -= artnetnode::configstore::DMXPORT_OFFSET;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= artnet::PORTS) {
			DEBUG_EXIT
			return;
		}

		uint16_t nMergeMode;
		ConfigStore::Get()->Copy(configstore::Store::NODE, &nMergeMode, sizeof(uint16_t), offsetof(struct artnetparams::Params, nMergeMode), false);

		nMergeMode &= artnetparams::mergemode_clear(nPortIndex);
		nMergeMode |= artnetparams::mergemode_set(nPortIndex, mergeMode);

		ConfigStore::Get()->Update(configstore::Store::NODE, offsetof(struct artnetparams::Params, nMergeMode), &nMergeMode, sizeof(uint16_t));

		DEBUG_EXIT
	}

	static void SavePortProtocol(uint32_t nPortIndex, const artnet::PortProtocol portProtocol) {
		DEBUG_ENTRY
		DEBUG_PRINTF("artnetnode::configstore::DMXPORT_OFFSET=%u, nPortIndex=%u, portProtocol=%u", artnetnode::configstore::DMXPORT_OFFSET, nPortIndex, static_cast<uint32_t>(portProtocol));

		if (nPortIndex >= artnetnode::configstore::DMXPORT_OFFSET) {
			nPortIndex -= artnetnode::configstore::DMXPORT_OFFSET;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= artnet::PORTS) {
			DEBUG_EXIT
			return;
		}

		uint16_t nPortProtocol;
		ConfigStore::Get()->Copy(configstore::Store::NODE, &nPortProtocol, sizeof(uint16_t), offsetof(struct artnetparams::Params, nProtocol), false);

		nPortProtocol &= artnetparams::protocol_clear(nPortIndex);
		nPortProtocol |= artnetparams::protocol_set(nPortIndex, portProtocol);

		ConfigStore::Get()->Update(configstore::Store::NODE, offsetof(struct artnetparams::Params, nProtocol), &nPortProtocol, sizeof(uint16_t));
		DEBUG_EXIT
	}

	static void SaveOutputStyle(uint32_t nPortIndex, const lightset::OutputStyle outputStyle) {
		DEBUG_ENTRY
		DEBUG_PRINTF("artnetnode::configstore::DMXPORT_OFFSET=%u, nPortIndex=%u, outputStyle=%u", artnetnode::configstore::DMXPORT_OFFSET, nPortIndex, static_cast<uint32_t>(outputStyle));

		if (nPortIndex >= artnetnode::configstore::DMXPORT_OFFSET) {
			nPortIndex -= artnetnode::configstore::DMXPORT_OFFSET;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= artnet::PORTS) {
			DEBUG_EXIT
			return;
		}

		uint8_t nOutputStyle;
		ConfigStore::Get()->Copy(configstore::Store::NODE, &nOutputStyle, sizeof(uint8_t), offsetof(struct artnetparams::Params, nOutputStyle), false);

		if (outputStyle == lightset::OutputStyle::CONSTANT) {
			nOutputStyle |= static_cast<uint8_t>(1U << nPortIndex);
		} else {
			nOutputStyle &= static_cast<uint8_t>(~(1U << nPortIndex));
		}

		ConfigStore::Get()->Update(configstore::Store::NODE, offsetof(struct artnetparams::Params, nOutputStyle), &nOutputStyle, sizeof(uint8_t));

		DEBUG_EXIT
	}

	static void SaveRdmEnabled(uint32_t nPortIndex, const bool isEnabled) {
		DEBUG_ENTRY
		DEBUG_PRINTF("artnetnode::configstore::DMXPORT_OFFSET=%u, nPortIndex=%u, isEnabled=%d", artnetnode::configstore::DMXPORT_OFFSET, nPortIndex, isEnabled);

		if (nPortIndex >= artnetnode::configstore::DMXPORT_OFFSET) {
			nPortIndex -= artnetnode::configstore::DMXPORT_OFFSET;
		} else {
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex >= artnet::PORTS) {
			DEBUG_EXIT
			return;
		}

		uint16_t nRdm;
		ConfigStore::Get()->Copy(configstore::Store::NODE, &nRdm, sizeof(uint16_t), offsetof(struct artnetparams::Params, nRdm), false);

		nRdm &= artnetparams::clear_mask(nPortIndex);

		if (isEnabled) {
			nRdm |= artnetparams::shift_left(1, nPortIndex);
			nRdm |= static_cast<uint16_t>(1U << (nPortIndex + 8));
		}

		ConfigStore::Get()->Update(configstore::Store::NODE, offsetof(struct artnetparams::Params, nRdm), &nRdm, sizeof(uint16_t));

		DEBUG_EXIT
	}

	static void SaveFailSafe(uint8_t nFailSafe) {
		DEBUG_ENTRY
		ConfigStore::Get()->Update(configstore::Store::NODE, offsetof(struct artnetparams::Params, nFailSafe), &nFailSafe, sizeof(uint8_t), artnetparams::Mask::FAILSAFE);
		DEBUG_EXIT
	}

private:
	void static SaveUniverse(uint32_t nPortIndex) {
		DEBUG_ENTRY
		DEBUG_PRINTF("artnetnode::configstore::DMXPORT_OFFSET=%u, nPortIndex=%u", artnetnode::configstore::DMXPORT_OFFSET, nPortIndex);

		uint16_t nUniverse;

		if (ArtNetNode::Get()->GetPortAddress(nPortIndex, nUniverse)) {
			if (nPortIndex >= artnetnode::configstore::DMXPORT_OFFSET) {
				nPortIndex -= artnetnode::configstore::DMXPORT_OFFSET;
			} else {
				DEBUG_EXIT
				return;
			}

			DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

			if (nPortIndex >= artnet::PORTS) {
				DEBUG_EXIT
				return;
			}

			DEBUG_PRINTF("nPortIndex=%u, nUniverse=%u", nPortIndex, nUniverse);
			ConfigStore::Get()->Update(configstore::Store::NODE, (sizeof(uint16_t) * nPortIndex) + offsetof(struct artnetparams::Params, nUniverse), &nUniverse, sizeof(uint16_t), artnetparams::Mask::UNIVERSE_A << nPortIndex);
		}

		DEBUG_EXIT
	}
};

#endif /* ARTNETSTORE_H_ */
