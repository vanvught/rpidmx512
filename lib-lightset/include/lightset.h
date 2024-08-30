/**
 * @file lightset.h
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LIGHTSET_H_
#define LIGHTSET_H_

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <climits>
#include <cassert>

namespace lightset {
namespace dmx {
static constexpr uint16_t ADDRESS_INVALID = 0xFFFF;
static constexpr uint32_t START_ADDRESS_DEFAULT = 1;
static constexpr uint32_t UNIVERSE_SIZE = 512;
static constexpr uint32_t MAX_VALUE = 255;
}
namespace node {
static constexpr uint32_t LABEL_NAME_LENGTH = 18;

inline void get_short_name_default(const uint32_t nPortIndex, char *pShortName) {
	snprintf(pShortName, node::LABEL_NAME_LENGTH - 1, "Port %u", static_cast<unsigned int>(1 + nPortIndex));
}
}  // namespace node

enum class MergeMode {
	HTP, LTP
};

enum class PortDir {
	INPUT, OUTPUT, DISABLE
};

enum class FailSafe {
	HOLD, OFF, ON, PLAYBACK
};

enum class OutputStyle {
	DELTA, CONSTANT
};

struct SlotInfo {
	uint16_t nCategory;
	uint8_t nType;
};

inline MergeMode get_merge_mode(const char *pMergeMode) {
	if (pMergeMode != nullptr) {
		if (((pMergeMode[0] | 0x20) == 'l')
		 && ((pMergeMode[1] | 0x20) == 't')
		 && ((pMergeMode[2] | 0x20) == 'p')) {
			return MergeMode::LTP;
		}
	}
	return MergeMode::HTP;
}

inline const char *get_merge_mode(const MergeMode mergeMode, const bool bToUpper = false) {
	if (bToUpper) {
		return (mergeMode == MergeMode::HTP) ? "HTP" : "LTP";
	}
	return (mergeMode == MergeMode::HTP) ? "htp" : "ltp";
}

inline const char *get_merge_mode(const unsigned m, const bool bToUpper = false) {
	return get_merge_mode(static_cast<MergeMode>(m), bToUpper);
}

inline PortDir get_direction(const char *pPortDir) {
	if (pPortDir != nullptr) {
		if (((pPortDir[0] | 0x20) == 'i')
		 && ((pPortDir[1] | 0x20) == 'n')
		 && ((pPortDir[2] | 0x20) == 'p')
		 && ((pPortDir[3] | 0x20) == 'u')
		 && ((pPortDir[4] | 0x20) == 't')) {
			return PortDir::INPUT;
		}
		if (((pPortDir[0] | 0x20) == 'd')
		 && ((pPortDir[1] | 0x20) == 'i')
		 && ((pPortDir[2] | 0x20) == 's')
		 && ((pPortDir[3] | 0x20) == 'a')
		 && ((pPortDir[4] | 0x20) == 'b')
		 && ((pPortDir[5] | 0x20) == 'l')
		 && ((pPortDir[6] | 0x20) == 'e')) {
			return PortDir::DISABLE;
		}
	}
	return PortDir::OUTPUT;
}

inline const char* get_direction(const PortDir portDir) {
	if (portDir == PortDir::INPUT) {
		return "input";
	}

	if (portDir == PortDir::DISABLE) {
		return "disable";
	}

	return "output";
}

inline FailSafe get_failsafe(const char *pFailSafe) {
	if (memcmp(pFailSafe, "hold", 4) == 0) {
		return FailSafe::HOLD;
	}

	if (memcmp(pFailSafe, "off", 3) == 0) {
		return FailSafe::OFF;
	}

	if (memcmp(pFailSafe, "on", 2) == 0) {
		return FailSafe::ON;
	}

	if (memcmp(pFailSafe, "playback", 8) == 0) {
		return FailSafe::PLAYBACK;
	}

	return FailSafe::HOLD;
}

inline const char* get_failsafe(const FailSafe failsafe) {
	if (failsafe == FailSafe::OFF) {
		return "off";
	} else if (failsafe == FailSafe::ON) {
		return "on";
	} else if (failsafe == FailSafe::PLAYBACK) {
		return "playback";
	}

	return "hold";
}

inline OutputStyle get_output_style(const char *pOutputStyle) {
	if (pOutputStyle != nullptr) {
		if (((pOutputStyle[0] | 0x20) == 'c')
		 && ((pOutputStyle[1] | 0x20) == 'o')
		 && ((pOutputStyle[2] | 0x20) == 'n')
		 && ((pOutputStyle[3] | 0x20) == 's')
		 && ((pOutputStyle[4] | 0x20) == 't')) {
			return OutputStyle::CONSTANT;
		}
	}
	return OutputStyle::DELTA;
}

inline const char *get_output_style(const OutputStyle outputStyle, const bool bToUpper = false) {
	if (bToUpper) {
		return (outputStyle == OutputStyle::DELTA) ? "DELTA" : "CONST";
	}
	return (outputStyle == OutputStyle::DELTA) ? "delta" : "const";
}

}  // namespace lightset

class LightSet {
public:
	LightSet() {}
	virtual ~LightSet() {}

	virtual void Start(const uint32_t nPortIndex)= 0;
	virtual void Stop(const uint32_t nPortIndex)= 0;
	virtual void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true)= 0;
	/**
	 * This is used for preparing the lightset output for a SYNC
	 * Typically used with DMX512 output
	 * @param [IN] PortIndex
	 */
	virtual void Sync(const uint32_t PortIndex)= 0;
	virtual void Sync()= 0;
#if defined (OUTPUT_HAVE_STYLESWITCH)
	virtual void SetOutputStyle(const uint32_t nPortIndex, const lightset::OutputStyle outputStyle)=0;
	virtual lightset::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const=0;
#endif
	// Optional
	virtual uint32_t GetRefreshRate() { return 0; }
	virtual void Blackout([[maybe_unused]] bool bBlackout) {}
	virtual void FullOn() {}
	virtual void Print() {}
	// RDM Optional
	virtual bool SetDmxStartAddress([[maybe_unused]] const uint16_t nDmxStartAddress) {
		return false;
	}
	virtual uint16_t GetDmxStartAddress() {
		return lightset::dmx::START_ADDRESS_DEFAULT;
	}
	virtual uint16_t GetDmxFootprint() {
		return lightset::dmx::UNIVERSE_SIZE;
	}
	virtual bool GetSlotInfo([[maybe_unused]] const uint16_t nSlotOffset, lightset::SlotInfo &slotInfo) {
		slotInfo.nType = 0x00; // ST_PRIMARY
		slotInfo.nCategory = 0x0001; // SD_INTENSITY
		return true;
	}
};

#endif /* LIGHTSET_H_ */
