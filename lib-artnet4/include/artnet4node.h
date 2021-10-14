/**
 * @file artnet4node.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNET4NODE_H_
#define ARTNET4NODE_H_

#include <cstdint>

#if !defined(ARTNET_VERSION)
# define ARTNET_VERSION	4
#endif

#include "artnetnode.h"
#include "artnet4handler.h"
#include "e131bridge.h"

class ArtNet4Node: public ArtNetNode, public ArtNet4Handler {
public:
	ArtNet4Node(uint8_t nPages = 1);

	void SetPort(uint32_t nPortIndex, lightset::PortDir dir) override;

	void HandleAddress(uint8_t nCommand) override;
	uint8_t GetStatus(uint32_t nPortIndex) override;

	bool IsStatusChanged() override {
		return m_Bridge.IsStatusChanged();
	}

	void Print();

	void Start();
	void Stop();
	void Run();

private:
	E131Bridge m_Bridge;
};

#endif /* ARTNET4NODE_H_ */
