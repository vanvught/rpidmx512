/**
 * @file dmxslotinfo.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DMXSLOTINFO_H_
#define DMXSLOTINFO_H_

#include <cstdint>

#include "lightset.h"

class DmxSlotInfo {
public:
	DmxSlotInfo(lightset::SlotInfo *pSlotInfo, uint32_t nSize);
	~DmxSlotInfo();

	void FromString(const char *pString, uint32_t &nMask);
	const char* ToString(uint32_t nMask);

	void Dump();

private:
	char* Parse(char *s, bool &isValid, lightset::SlotInfo &tLightSetSlotInfo);

private:
	lightset::SlotInfo *m_pSlotInfo;
	uint32_t m_nSize;
	char *m_pToString { nullptr };
};

#endif /* DMXSLOTINFO_H_ */
