/**
 * @file artnet4node.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "artnetnode.h"
#include "artnet4handler.h"
#include "e131bridge.h"

class ArtNet4Node: public ArtNetNode, public ArtNet4Handler {
public:
	ArtNet4Node(uint8_t nPages = 1);

	void SetPort(uint8_t nPortId, TArtNetPortDir dir = ARTNET_OUTPUT_PORT) override;

	void Print();

	void Start();
	void Stop();
	void Run();

	void HandleAddress(uint8_t nCommand) override;
	uint8_t GetStatus(uint8_t nPortId) override;

	void SetMapUniverse0(bool bMapUniverse0 = false) {
		m_bMapUniverse0 = bMapUniverse0;
	}
	bool IsMapUniverse0() {
		return m_bMapUniverse0;
	}

	bool IsStatusChanged() override {
		return m_Bridge.IsStatusChanged();
	}

private:
	E131Bridge m_Bridge;
	bool m_bMapUniverse0{false};
};

#endif /* ARTNET4NODE_H_ */
