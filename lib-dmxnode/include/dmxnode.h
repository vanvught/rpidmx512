/**
 * @file dmxnode.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXNODE_H_
#define DMXNODE_H_

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

namespace dmxnode {
static constexpr uint16_t ADDRESS_INVALID = 0xFFFF;
static constexpr uint32_t START_ADDRESS_DEFAULT = 1;
static constexpr uint32_t UNIVERSE_SIZE = 512;
static constexpr uint32_t DMX_MAX_VALUE = 255;
/*
 * Art-Net
 */
static constexpr uint32_t NODE_NAME_LENGTH  = 64;
static constexpr uint32_t LABEL_NAME_LENGTH = 18;
/*
 * sACN E1.31
 */
static constexpr uint8_t PRIORITY_LOWEST  = 1;
static constexpr uint8_t PRIORITY_DEFAULT = 100;
static constexpr uint8_t PRIORITY_HIGHEST = 200;

#if !defined(DMXNODE_PORTS)
static constexpr uint32_t MAX_PORTS = 1;
#else
static constexpr uint32_t MAX_PORTS = DMXNODE_PORTS;
#endif

#if !defined(DMXNODE_PARAM_PORTS)
static constexpr uint32_t PARAM_PORTS = 4;
#else
static constexpr uint32_t PARAM_PORTS = DMXNODE_PARAM_PORTS;
#endif

#if !defined(CONFIG_DMXNODE_DMX_PORT_OFFSET)
static constexpr uint32_t DMXPORT_OFFSET = 0;
#else
static constexpr uint32_t DMXPORT_OFFSET = CONFIG_DMXNODE_DMX_PORT_OFFSET;
#endif

static constexpr uint32_t CONFIG_PORT_COUNT =
    ((MAX_PORTS - DMXPORT_OFFSET) <= PARAM_PORTS) ? (MAX_PORTS - DMXPORT_OFFSET) : PARAM_PORTS;


enum class Personality {
	NODE, ARTNET, SACN
};

enum class MergeMode {
	HTP, LTP
};

enum class PortDirection {
	INPUT, OUTPUT, DISABLE
};

enum class FailSafe {
	HOLD, OFF, ON, PLAYBACK, RECORD
};

enum class OutputStyle {
	DELTA, CONSTANT
};

enum class Rdm {
	DISABLE, ENABLE
};

struct SlotInfo {
	uint16_t nCategory;
	uint8_t nType;
};

inline Personality get_personality(const char *pPersonality) {
	assert(pPersonality != nullptr);
	if (strncasecmp(pPersonality, "node", 4) == 0) {
		return Personality::NODE;
	}

	if (strncasecmp(pPersonality, "sacn", 4) == 0) {
		return Personality::SACN;
	}

	return Personality::ARTNET;
}

inline const char *get_personality(const Personality personality) {
	if (personality == Personality::NODE) {
		return "node";
	}

	if (personality == Personality::SACN) {
		return "sacn";
	}

	return "artnet";
}

inline MergeMode get_merge_mode(const char *pMergeMode) {
	assert(pMergeMode != nullptr);
	if (strncasecmp(pMergeMode, "ltp", 3) == 0) {
		return MergeMode::LTP;
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

inline PortDirection get_port_direction(const char *pPortDirection) {
	assert(pPortDirection != nullptr);

	if (strncasecmp(pPortDirection, "input", 5) == 0) {
		return PortDirection::INPUT;
	}

	if (strncasecmp(pPortDirection, "disable", 7) == 0) {
		return PortDirection::DISABLE;
	}

	return PortDirection::OUTPUT;
}

inline const char *get_port_direction(const PortDirection portDirection) {
	if (portDirection == PortDirection::INPUT) {
		return "input";
	}

	if (portDirection == PortDirection::DISABLE) {
		return "disable";
	}

	return "output";
}

inline FailSafe get_failsafe(const char *pFailSafe) {
	if (strncasecmp(pFailSafe, "hold", 4) == 0) {
		return FailSafe::HOLD;
	}

	if (strncasecmp(pFailSafe, "off", 3) == 0) {
		return FailSafe::OFF;
	}

	if (strncasecmp(pFailSafe, "on", 2) == 0) {
		return FailSafe::ON;
	}

	if (strncasecmp(pFailSafe, "playback", 8) == 0) {
		return FailSafe::PLAYBACK;
	}

	return FailSafe::HOLD;
}

inline const char *get_failsafe(const FailSafe failsafe) {
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
	assert(pOutputStyle != nullptr);
	if (strncasecmp(pOutputStyle, "const", 5) == 0) {
		return OutputStyle::CONSTANT;
	}

	return OutputStyle::DELTA;
}

inline const char *get_output_style(const OutputStyle outputStyle, const bool bToUpper = false) {
	if (bToUpper) {
		return (outputStyle == OutputStyle::DELTA) ? "DELTA" : "CONST";
	}
	return (outputStyle == OutputStyle::DELTA) ? "delta" : "const";
}

inline void get_short_name_default(const uint32_t nPortIndex, char *pShortName) {
	snprintf(pShortName, LABEL_NAME_LENGTH - 1, "Port %u", static_cast<unsigned int>(1 + nPortIndex));
}
}  // namespace dmxnode

#endif /* DMXNODE_H_ */
