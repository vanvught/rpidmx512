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

#if !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
#endif

#include <cstdint>
#include "formats/showfileformatola.h"

#if defined (CONFIG_SHOWFILE_PROTOCOL_NODE_ARTNET)
#include "artnet.h"

namespace showfile {
void record(const struct artnet::ArtDmx *pArtDmx, const uint32_t nMillis) {
	const auto nDmxSlots = static_cast<uint32_t>(((pArtDmx->LengthHi << 8) & 0xff00) | pArtDmx->Length);
	ShowFileFormat::Get()->ShowfileWrite(pArtDmx->Data, nDmxSlots, pArtDmx->PortAddress, nMillis);
}

void record([[maybe_unused]] const struct artnet::ArtSync *pArtSync, [[maybe_unused]] const uint32_t nMillis) {
	// Nothng to do here
}
}  // namespace showfile
#endif

#if defined (CONFIG_SHOWFILE_PROTOCOL_NODE_E131)
#include "e131packets.h"

namespace showfile {
void record(const struct TE131DataPacket *pE131DataPacket, const uint32_t nMillis) {
	const auto *const pDmxData = &pE131DataPacket->DMPLayer.PropertyValues[1];
	const auto nLength = __builtin_bswap16(pE131DataPacket->DMPLayer.PropertyValueCount) - 1U;
	const auto Universe = __builtin_bswap16(pE131DataPacket->FrameLayer.Universe);
	ShowFileFormat::Get()->ShowfileWrite(pDmxData, nLength, Universe, nMillis);
}

void record([[maybe_unused]] const struct TE131SynchronizationPacket *pE131SynchronizationPacke, [[maybe_unused]] const uint32_t nMillis) {
	// Nothing to do here
}
}  // namespace showfile
#endif
