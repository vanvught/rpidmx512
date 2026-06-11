/**
 * @file tlc59711dmx.cpp
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cassert>

#include "tlc59711dmx.h"
#include "tlc59711dmxstore.h"
#include "tlc59711.h"
#include "dmxnode.h"
#include "firmware/debug/debug_debug.h"

TLC59711Dmx::TLC59711Dmx() {
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    UpdateMembers();

    DEBUG_EXIT();
}

TLC59711Dmx::~TLC59711Dmx() {
    DEBUG_ENTRY();

    if (tlc59711_ != nullptr) {
        delete tlc59711_;
        tlc59711_ = nullptr;
    }
#if defined(CONFIG_TLC59711DMX_ENABLE_PCT)
    if (m_ArrayMaxValue != nullptr) {
        delete[] m_ArrayMaxValue;
        m_ArrayMaxValue = nullptr;
    }
#endif

    DEBUG_EXIT();
}

void TLC59711Dmx::Start([[maybe_unused]] uint32_t port_index) {
    if (started_) {
        assert(m_pTLC59711 != nullptr);
        return;
    }

    started_ = true;

    if (tlc59711_ == nullptr) {
        Initialize();
    }
}

void TLC59711Dmx::Stop([[maybe_unused]] uint32_t port_index) {
    if (!started_) {
        return;
    }

    started_ = false;
}

void TLC59711Dmx::Initialize() {
   const auto kBoardInstances = static_cast<uint8_t>((dmx_footprint_ + (TLC59711Channels::kOut - 1)) / TLC59711Channels::kOut);

    assert(m_pTLC59711 == nullptr);
    tlc59711_ = new TLC59711(kBoardInstances, spi_speed_hz_);
    assert(m_pTLC59711 != nullptr);

#if defined(CONFIG_TLC59711DMX_ENABLE_PCT)
    assert(m_ArrayMaxValue == nullptr);
    m_ArrayMaxValue = new uint16_t[dmx_footprint_];
    assert(m_ArrayMaxValue != nullptr);

    for (uint32_t nIndex = 0; nIndex < dmx_footprint_; nIndex++) {
        m_ArrayMaxValue[nIndex] = UINT16_MAX;
    }
#endif

    tlc59711_->Dump();
}

#if defined(CONFIG_TLC59711DMX_ENABLE_PCT)
void TLC59711Dmx::SetMaxPct(uint32_t nIndexLed, uint32_t nPct) {
    assert(nIndexLed < count_);

    if (nPct > 100) {
        nPct = 100;
    }

    const auto nMaxValue = static_cast<uint16_t>((UINT16_MAX * nPct) / 100U);

    if (type_ == tlc59711::Type::RGB) {
        const auto nIndexArray = nIndexLed * 3U;
        for (uint32_t i = 0; i < 3; i++) {
            m_ArrayMaxValue[i + nIndexArray] = nMaxValue;
        }
    } else {
        const auto nIndexArray = nIndexLed * 4U;
        for (uint32_t i = 0; i < 4; i++) {
            m_ArrayMaxValue[i + nIndexArray] = nMaxValue;
        }
    }

#if 0
	for (uint32_t nIndexLed = 0; nIndexLed < count_; nIndexLed++) {
		if (type_ == tlc59711::Type::RGB) {
			const auto nIndexArray = nIndexLed * 3U;
			for (uint32_t i = 0; i < 3; i++) {
				printf("%.4x ", m_ArrayMaxValue[i + nIndexArray]);
			}
			puts("");
		} else {
			const auto nIndexArray = nIndexLed * 4U;
			for (uint32_t i = 0; i < 4; i++) {
				printf("%.4x ", m_ArrayMaxValue[i + nIndexArray]);
			}
			puts("");
		}
	}
#endif
}
#endif

template <bool doUpdate> 
void TLC59711Dmx::SetData([[maybe_unused]] uint32_t port_index, const uint8_t* data, uint32_t length) {
    assert(pDmxData != nullptr);
    assert(nLength <= dmxnode::kUniverseSize);

    if (__builtin_expect((tlc59711_ == nullptr), 0)) {
        Start();
    }

    auto* p = const_cast<uint8_t*>(data) + dmx_start_address_ - 1;
    auto dmx_address = dmx_start_address_;

    for (uint32_t i = 0; i < dmx_footprint_; i++) {
        if (dmx_address > length) {
            break;
        }

        auto value = static_cast<uint16_t>((*p << 8) | *p);

#if defined(CONFIG_TLC59711DMX_ENABLE_PCT)
        if (value > m_ArrayMaxValue[i]) {
            value = m_ArrayMaxValue[i];
        }
#endif
        tlc59711_->Set(i, value); // NOLINT(clang-analyzer-core.CallAndMessage): Start() ensures m_pTLC59711 is not nullptr

        p++;
        dmx_address++;
    }

    if (__builtin_expect((dmx_address == dmx_start_address_), 0)) {
        return;
    }

    if constexpr (doUpdate) {
        if (!blackout_) {
            tlc59711_->Update();
        }
    }
}

void TLC59711Dmx::Sync([[maybe_unused]] uint32_t port_index) {
    // No actions here
}

void TLC59711Dmx::Sync() {
    if (!blackout_) {
        tlc59711_->Update();
    }
}

void TLC59711Dmx::SetType(tlc59711::Type type) {
    type_ = type;
    UpdateMembers();
}

void TLC59711Dmx::SetCount(uint32_t count) {
    count_ = static_cast<uint16_t>(count);
    UpdateMembers();
}

void TLC59711Dmx::SetSpiSpeedHz(uint32_t spi_speed_hz) {
    spi_speed_hz_ = spi_speed_hz;
}

void TLC59711Dmx::UpdateMembers() {
    if (type_ == tlc59711::Type::kRgb) {
        dmx_footprint_ = count_ * 3U;
    } else {
        dmx_footprint_ = count_ * 4U;
    }
}

void TLC59711Dmx::Blackout(bool blackout) {
    blackout_ = blackout;

    if (blackout) {
        tlc59711_->Blackout();
    } else {
        tlc59711_->Update();
    }
}

// DMX

bool TLC59711Dmx::SetDmxStartAddress(uint16_t dmx_start_address) {
    assert((dmx_start_address != 0) && (dmx_start_address <= (dmxnode::kUniverseSize - dmx_footprint_)));

    if ((dmx_start_address != 0) && (dmx_start_address <= (dmxnode::kUniverseSize - dmx_footprint_))) {
        dmx_start_address_ = dmx_start_address;
        tlc59711dmx_store::SaveDmxStartAddress(dmx_start_address_);
        return true;
    }

    return false;
}

// RDM

bool TLC59711Dmx::GetSlotInfo(uint16_t slot_offset, dmxnode::SlotInfo& slot_info) {
    if (slot_offset > dmx_footprint_) {
        return false;
    }

    uint32_t index;

    if (type_ == tlc59711::Type::kRgb) {
        index = slot_offset % 3U;
    } else {
        index = slot_offset % 4U;
    }

    slot_info.type = 0x00; // ST_PRIMARY

    switch (index) {
        case 0:
            slot_info.category = 0x0205; // SD_COLOR_ADD_RED
            break;
        case 1:
            slot_info.category = 0x0206; // SD_COLOR_ADD_GREEN
            break;
        case 2:
            slot_info.category = 0x0207; // SD_COLOR_ADD_BLUE
            break;
        case 3:
            slot_info.category = 0x0212; // SD_COLOR_ADD_WHITE
            break;
        default:
            assert(0);
            break;
    }

    return true;
}

void TLC59711Dmx::Print() {
    puts("TLC59711 parameters");
    printf(" Type  : %s [%d]\n", tlc59711::GetType(type_), static_cast<uint32_t>(type_));
    printf(" Count : %d %s\n", count_, type_ == tlc59711::Type::kRgb ? "RGB" : "RGBW");
    printf(" Clock : %d Hz %s {Default: %d Hz, Maximum %d Hz}\n", spi_speed_hz_, (spi_speed_hz_ == 0 ? "Default" : ""), TLC59711SpiSpeed::kDefault, TLC59711SpiSpeed::kMax);
    printf(" DMX   : StartAddress=%d, FootPrint=%d\n", dmx_start_address_, dmx_footprint_);
}

// Explicit template instantiations
template void TLC59711Dmx::SetData<true>(uint32_t, const uint8_t*, uint32_t);
template void TLC59711Dmx::SetData<false>(uint32_t, const uint8_t*, uint32_t);
