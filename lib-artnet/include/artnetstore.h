/**
 * @file artnetstore.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETSTORE_H_
#define ARTNETSTORE_H_

#include <cstdint>

#include "artnet.h"
#include "lightset.h"

class ArtNetStore {
public:
	virtual ~ArtNetStore() {}

	virtual void SaveLongName(const char *pLongName)=0;

	virtual void SaveShortName(const uint32_t nPortIndex, const char *pShortName)=0;

	virtual void SaveUniverseSwitch(const uint32_t nPortIndex, const uint8_t nAddress)=0;
	virtual void SaveNetSwitch(const uint32_t nPage, const uint8_t nAddress)=0;
	virtual void SaveSubnetSwitch(const uint32_t nPage, const uint8_t nAddress)=0;

	virtual void SaveMergeMode(const uint32_t nPortIndex, const lightset::MergeMode mergeMode)=0;
	virtual void SavePortProtocol(const uint32_t nPortIndex, const artnet::PortProtocol portProtocol)=0;
	virtual void SaveOutputStyle(const uint32_t nPortIndex, const lightset::OutputStyle outputStyle)=0;
	virtual void SaveRdmEnabled(const uint32_t nPortIndex, const bool isEnabled)=0;

	virtual void SaveFailSafe(const uint8_t nFailsafe)=0;
};

#endif /* ARTNETSTORE_H_ */
