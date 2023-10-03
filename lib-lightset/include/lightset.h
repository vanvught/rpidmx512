/**
 * @file lightset.h
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
	snprintf(pShortName, node::LABEL_NAME_LENGTH - 1, "Port %u", 1U + nPortIndex);
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

#if defined (ESP8266)
enum class OutputType {
	DMX, SPI, MONITOR, UNDEFINED
};
#endif

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

inline const char* get_merge_mode(const MergeMode mergeMode, const bool bToUpper = false) {
	if (bToUpper) {
		return (mergeMode == MergeMode::HTP) ? "HTP" : "LTP";
	}
	return (mergeMode == MergeMode::HTP) ? "htp" : "ltp";
}

inline const char* get_merge_mode(const unsigned m, const bool bToUpper = false) {
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
	switch (failsafe) {
	case FailSafe::HOLD:
		return "hold";
		break;
	case FailSafe::OFF:
		return "off";
		break;
	case FailSafe::ON:
		return "on";
		break;
	case FailSafe::PLAYBACK:
		return "playback";
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	__builtin_unreachable();
	return "";
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
	virtual void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true)= 0;
	/**
	 * This is used for preparing the lightset output for a SYNC
	 * Typically used with DMX512 output
	 * @param [IN] PortIndex
	 */
	virtual void Sync(const uint32_t PortIndex)= 0;
	/**
	 *
	 * @param [IN] doForce This parameter is used with Art-Net ArtSync only.
	 */
	virtual void Sync(const bool doForce = false)= 0;
#if defined (OUTPUT_HAVE_STYLESWITCH)
	virtual void SetOutputStyle(const uint32_t nPortIndex, const lightset::OutputStyle outputStyle)=0;
	virtual lightset::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const=0;
#endif
	// Optional
	virtual void Blackout(__attribute__((unused)) bool bBlackout) {}
	virtual void FullOn() {}
	virtual void Print() {}
	// RDM Optional
	virtual bool SetDmxStartAddress(uint16_t nDmxStartAddress);
	virtual uint16_t GetDmxStartAddress();
	virtual uint16_t GetDmxFootprint();
	virtual bool GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo);
#if defined (LIGHTSET_HAVE_RUN)
	virtual void Run()= 0;
#endif
#if defined (ESP8266)
	static const char *GetOutputType(lightset::OutputType type);
	static lightset::OutputType GetOutputType(const char *sType);
#endif
};

#endif /* LIGHTSET_H_ */
