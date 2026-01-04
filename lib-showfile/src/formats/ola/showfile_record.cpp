/**
 * @file showfile_record.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
#endif

#include <cstdint>
#include "formats/showfileformatola.h"

#if defined (CONFIG_SHOWFILE_PROTOCOL_NODE_ARTNET)
#include "artnet.h"

namespace showfile {
void record(const struct artnet::ArtDmx *pArtDmx, uint32_t millis) {
	const auto kDmxSlots = static_cast<uint32_t>(((pArtDmx->LengthHi << 8) & 0xff00) | pArtDmx->Length);
	ShowFileFormat::Get()->ShowfileWrite(pArtDmx->data, kDmxSlots, pArtDmx->PortAddress, millis);
}

void record([[maybe_unused]] const struct artnet::ArtSync *pArtSync, [[maybe_unused]] uint32_t millis) {
	// Nothng to do here
}
}  // namespace showfile
#endif

#if defined (CONFIG_SHOWFILE_PROTOCOL_NODE_E131)
#include "e131.h"

namespace showfile {
void record(const struct e131::DataPacket *pE131DataPacket, uint32_t millis) {
	const auto *const pDmxData = &pE131DataPacket->dmp_layer.property_values[1];
	const auto nLength = __builtin_bswap16(pE131DataPacket->dmp_layer.property_value_count) - 1U;
	const auto Universe = __builtin_bswap16(pE131DataPacket->frame_layer.universe);
	ShowFileFormat::Get()->ShowfileWrite(pDmxData, nLength, Universe, millis);
}

void record([[maybe_unused]] const struct e131::SynchronizationPacket *pE131SynchronizationPacke, [[maybe_unused]] uint32_t millis) {
	// Nothing to do here
}
}  // namespace showfile
#endif
