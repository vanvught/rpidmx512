/**
 * @file artnetnodegetportaddress.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#include "lightset.h"

bool ArtNetNode::GetPortAddress(uint32_t nPortIndex, uint16_t& nAddress) const {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
		nAddress = m_InputPort[nPortIndex].genericPort.nPortAddress;
		return true;
	}

	if (m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
		nAddress = m_OutputPort[nPortIndex].genericPort.nPortAddress;
		return true;
	}

	return false;
}

bool ArtNetNode::GetPortAddress(uint32_t nPortIndex, uint16_t& nAddress, lightset::PortDir dir) const {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	if (dir == lightset::PortDir::INPUT) {
		nAddress = m_InputPort[nPortIndex].genericPort.nPortAddress;
		return m_InputPort[nPortIndex].genericPort.bIsEnabled;
	}

	if (dir == lightset::PortDir::OUTPUT) {
		nAddress = m_OutputPort[nPortIndex].genericPort.nPortAddress;
		return m_OutputPort[nPortIndex].genericPort.bIsEnabled;
	}

	return false;
}
