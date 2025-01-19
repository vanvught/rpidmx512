/**
 * @file gpsparams.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef GPSPARAMS_H_
#define GPSPARAMS_H_

#include <cstdint>

#include "gpsconst.h"
#include "configstore.h"

namespace gpsparams {
struct Params {
	uint32_t nSetList;
	uint8_t nModule;
	float fUtcOffset;
} __attribute__((packed));

struct Mask {
	static constexpr auto MODULE = (1U << 0);
	static constexpr auto ENABLE = (1U << 1);
	static constexpr auto UTC_OFFSET = (1U << 2);
};
}  // namespace gpsparams

class GPSParamsStore  {
public:
	static void Update(const struct gpsparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::GPS, pParams, sizeof(struct gpsparams::Params));
	}

	static void Copy(struct gpsparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::GPS, pParams, sizeof(struct gpsparams::Params));
	}
};

class GPSParams {
public:
	GPSParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct gpsparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	bool IsEnabled() const {
		return isMaskSet(gpsparams::Mask::ENABLE);
	}

	GPSModule GetModule() const {
		return static_cast<GPSModule>(m_Params.nModule);
	}

	float GetUtcOffset() const {
		return m_Params.fUtcOffset;
	}

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_Params.nSetList & nMask) == nMask;
	}

private:
	gpsparams::Params m_Params;
};

#endif /* GPSPARAMS_H_ */
