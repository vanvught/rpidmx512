/**
 * @file bwspidimmer.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef BWSPIDIMMER_H_
#define BWSPIDIMMER_H_

#include <cstdint>

#include "bw.h"

class BwSpiDimmer: BwSpi {
public:
	BwSpiDimmer(uint8_t nChipSelect = 0, uint8_t nAddress = bw::dimmer::address): BwSpi(nChipSelect, nAddress, bw::dimmer::id_string) {}

	void Output(uint8_t nValue) {
		char cmd[3];

		cmd[0] = static_cast<char>(m_nAddress);
		cmd[1] = bw::port::write::set_all_outputs;
		cmd[2] = static_cast<char>(nValue);

		HAL_SPI::Write(cmd, sizeof(cmd));
	}

	bool IsConnected() {
		return m_IsConnected;
	}
};

#endif /* BWSPIDIMMER_H_ */
