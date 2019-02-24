/**
 * @file storeartnet.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef STOREARTNET_H_
#define STOREARTNET_H_

#include "artnetparams.h"
#include "artnetstore.h"

class StoreArtNet: public ArtNetParamsStore, ArtNetStore {
public:
	StoreArtNet(void);
	~StoreArtNet(void);

	void Update(const struct TArtNetParams *pArtNetParams);
	void Copy(struct TArtNetParams *pArtNetParams);

	void SaveShortName(const char *pShortName);
	void SaveLongName(const char *pLongName);
	void SaveUniverseSwitch(uint8_t nPortIndex, uint8_t nAddress);
	void SaveNetSwitch(uint8_t nAddress);
	void SaveSubnetSwitch(uint8_t nAddress);
	void SaveMergeMode(uint8_t nPortIndex, TMerge tMerge);
	void SavePortProtocol(uint8_t nPortIndex, TPortProtocol tPortProtocol);
};

#endif /* STOREARTNET_H_ */
