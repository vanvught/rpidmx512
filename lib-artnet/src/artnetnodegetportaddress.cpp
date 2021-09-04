/**
 * @file artnetnodegetportaddress.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "artnet.h"

using namespace artnet;

bool ArtNetNode::GetPortAddress(uint32_t nPortIndex, uint16_t &nAddress, PortDir dir) const {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	if (dir == PortDir::INPUT) {
		nAddress = m_InputPorts[nPortIndex].port.nPortAddress;
		return m_InputPorts[nPortIndex].bIsEnabled;
	}

	if (dir == PortDir::OUTPUT) {
		nAddress = m_OutputPorts[nPortIndex].port.nPortAddress;
		return m_OutputPorts[nPortIndex].bIsEnabled;
	}

	return false;
}
