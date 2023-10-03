/**
 * @file artnetnodefailsafe.cpp
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

#include <cstdint>
#include <cassert>

#include "artnetnode.h"
#include "artnetnodefailsafe.h"
#include "lightsetdata.h"

#include "debug.h"

void ArtNetNode::FailSafeRecord() {
	DEBUG_ENTRY

	artnetnode::failsafe_write_start();

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
			artnetnode::failsafe_write(nPortIndex, lightset::Data::Backup(nPortIndex));
		}
	}

	artnetnode::failsafe_write_end();

	DEBUG_EXIT
}

void ArtNetNode::FailSafePlayback() {
	DEBUG_ENTRY

	artnetnode::failsafe_read_start();

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
			artnetnode::failsafe_read(nPortIndex, const_cast<uint8_t *>(lightset::Data::Backup(nPortIndex)));
			lightset::Data::Output(m_pLightSet, nPortIndex);

			if (!m_OutputPort[nPortIndex].IsTransmitting) {
				m_pLightSet->Start(nPortIndex);
				m_OutputPort[nPortIndex].IsTransmitting = true;
			}

			lightset::Data::ClearLength(nPortIndex);
		}
	}

	artnetnode::failsafe_read_end();

	DEBUG_EXIT
}
