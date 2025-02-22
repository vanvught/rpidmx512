/**
 * @file dmxmonitor.h
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXMONITOR_H_
#define DMXMONITOR_H_

#include <cstdint>

#include "dmxnode.h"

#include "debug.h"

namespace dmxmonitor {
enum class Format {
	HEX, PCT, DEC,
};
namespace output {
namespace hdmi {
static constexpr char MAX_PORTS = 1;
}  // namespace hdmi
namespace text {
#if !defined(DMXNODE_PORTS)
 static constexpr char MAX_PORTS = 4;
#else
 static constexpr char MAX_PORTS = DMXNODE_PORTS;
#endif
}  // namespace text
}  // namespace output
}  // namespace dmxmonitor

class DMXMonitor {
public:
	DMXMonitor();
	~DMXMonitor()  = default;

	void Print()  {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void Start(const uint32_t nPortIndex) ;
	void Stop(const uint32_t nPortIndex) ;

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) ;
	void Sync([[maybe_unused]] const uint32_t nPortIndex)  {
#if defined (__linux__) || defined(__APPLE__)
		Update(nPortIndex, m_Data[nPortIndex].data, m_Data[nPortIndex].nLength);
#else
		Update();
#endif
	}

	void Sync()  {}

	uint32_t GetUserData() { return 0; }
	uint32_t GetRefreshRate() { return 0; }

	void Blackout([[maybe_unused]] bool bBlackout)  {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void FullOn()  {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(const uint32_t nPortIndex, const dmxnode::OutputStyle outputStyle)  {
		DEBUG_ENTRY

		assert(nPortIndex < 32);
		m_nOutPutStyle = static_cast<uint32_t>(outputStyle) << nPortIndex;

		DEBUG_EXIT
	}

	dmxnode::OutputStyle GetOutputStyle([[maybe_unused]] const uint32_t nPortIndex) const  {
		DEBUG_ENTRY
#if defined (__linux__) || defined(__APPLE__)
		assert(nPortIndex < 32);
		return static_cast<dmxnode::OutputStyle>(m_nOutPutStyle >> nPortIndex);
#else
		return dmxnode::OutputStyle::DELTA;
#endif
		DEBUG_EXIT
	}
#endif

	bool SetDmxStartAddress(uint16_t nDmxStartAddress) ;
	uint16_t GetDmxStartAddress()  {
		return m_nDmxStartAddress;
	}

	uint16_t GetDmxFootprint()  {
#if defined (__linux__) || defined(__APPLE__)
		return m_nMaxChannels;
#else
		return dmxnode::UNIVERSE_SIZE;
#endif
	}

	bool GetSlotInfo([[maybe_unused]] const uint16_t nSlotOffset, dmxnode::SlotInfo &slotInfo) {
		slotInfo.nType = 0x00; // ST_PRIMARY
		slotInfo.nCategory = 0x0001; // SD_INTENSITY
		return true;
	}

	void SetFormat(const dmxmonitor::Format format) {
		m_Format = format;
	}

	dmxmonitor::Format GetFormat() const {
		return m_Format;
	}

#if defined (__linux__) || defined(__APPLE__)
	void Cls() {}
#else
	void Cls();
#endif

#if defined (__linux__) || defined(__APPLE__)
	void SetMaxDmxChannels(uint16_t nMaxChannels) {
		m_nMaxChannels = nMaxChannels;
	}

private:
	void DisplayDateTime(const uint32_t nPortIndex, const char *pString);
	void Update(const uint32_t nPortIndex, const uint8_t *pData, const uint32_t nLength);
#else
private:
	void Update();
#endif

private:
	dmxmonitor::Format m_Format { dmxmonitor::Format::HEX };
	uint16_t m_nDmxStartAddress { dmxnode::START_ADDRESS_DEFAULT };
#if defined (OUTPUT_HAVE_STYLESWITCH)
	uint32_t m_nOutPutStyle;
#endif
#if defined (__linux__) || defined(__APPLE__)
	enum {
		DMX_DEFAULT_MAX_CHANNELS = 32,
	};
	bool m_bIsStarted[dmxmonitor::output::text::MAX_PORTS];
	uint16_t m_nMaxChannels { DMX_DEFAULT_MAX_CHANNELS };
	struct Data {
		uint8_t data[516];
		uint32_t nLength;
	};
	struct Data m_Data[dmxmonitor::output::text::MAX_PORTS];
#else
	uint16_t m_nSlots { 0 };
	bool m_bIsStarted { false };
	uint8_t m_Data[512];
#endif
};

#endif /* DMXMONITOR_H_ */
