/**
 * @file dmxmonitor.h
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

#ifndef DMXMONITOR_H_
#define DMXMONITOR_H_

#include <cstdint>

#include "dmxmonitorstore.h"
#include "lightset.h"

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
#if !defined(LIGHTSET_PORTS)
 static constexpr char MAX_PORTS = 4;
#else
 static constexpr char MAX_PORTS = LIGHTSET_PORTS;
#endif
}  // namespace text
}  // namespace output
}  // namespace dmxmonitor

class DMXMonitor: public LightSet {
public:
	DMXMonitor();
	~DMXMonitor() override {}

	void Print() override {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void Start(const uint32_t nPortIndex) override;
	void Stop(const uint32_t nPortIndex) override;

	void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) override;
	void Sync(const uint32_t nPortIndex) override;
	void Sync(const bool doForce = false) override;

	void Blackout(__attribute__((unused)) bool bBlackout) override {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void FullOn() override {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(const uint32_t nPortIndex, const lightset::OutputStyle outputStyle) override {
		DEBUG_ENTRY

		assert(nPortIndex < 32);
		m_nOutPutStyle = static_cast<uint32_t>(outputStyle) << nPortIndex;

		DEBUG_EXIT
	}

	lightset::OutputStyle GetOutputStyle(__attribute__((unused)) const uint32_t nPortIndex) const override {
		DEBUG_ENTRY

#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
		assert(nPortIndex < 32);
		return static_cast<lightset::OutputStyle>(m_nOutPutStyle >> nPortIndex);
#else
		return lightset::OutputStyle::DELTA;
#endif
		DEBUG_EXIT
	}
#endif

	bool SetDmxStartAddress(uint16_t nDmxStartAddress) override;
	uint16_t GetDmxStartAddress() override;

	uint16_t GetDmxFootprint() override;

	void SetFormat(const dmxmonitor::Format format) {
		m_tFormat = format;
	}

	dmxmonitor::Format GetFormat() const {
		return m_tFormat;
	}

	void Cls();

	void SetDmxMonitorStore(DmxMonitorStore *pDmxMonitorStore) {
		m_pDmxMonitorStore = pDmxMonitorStore;
	}

#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
	void SetMaxDmxChannels(uint16_t nMaxChannels);

private:
	void DisplayDateTime(uint32_t nPortIndex, const char *pString);
#endif

private:
	void Update();
	void Update(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength);

private:
	dmxmonitor::Format m_tFormat = dmxmonitor::Format::HEX;
	uint32_t m_nSlots { 0 };
	uint32_t m_nOutPutStyle;
	DmxMonitorStore *m_pDmxMonitorStore { nullptr };
#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
	enum {
		DMX_DEFAULT_MAX_CHANNELS = 32,
	};
	bool m_bIsStarted[dmxmonitor::output::text::MAX_PORTS];
	uint16_t m_nDmxStartAddress { lightset::dmx::START_ADDRESS_DEFAULT };
	uint16_t m_nMaxChannels { DMX_DEFAULT_MAX_CHANNELS };
	struct Data {
		uint8_t data[516];
		uint32_t nLength;
	};
	struct Data m_Data[dmxmonitor::output::text::MAX_PORTS];
#else
	bool m_bIsStarted { false };
	uint8_t m_Data[512];
#endif
};

#endif /* DMXMONITOR_H_ */
