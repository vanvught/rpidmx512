/**
 * @file fotaparams.h
 *
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

#ifndef FOTAPARAMS_H_
#define FOTAPARAMS_H_

#include <cstdint>

struct TFotaParams {
	uint32_t nSetList;
	uint32_t nServerIp;
}__attribute__((packed));

static_assert(sizeof(struct TFotaParams) <= 32, "struct TFotaParams is too large");

struct FotaParamsMask {
	static constexpr auto SERVER = (1U << 0);
};

class FotaParamsStore {
public:
	virtual ~FotaParamsStore() {
	}

	virtual void Update(const struct TFotaParams *pFotaParams)=0;
	virtual void Copy(struct TFotaParams *pFotaParams)=0;
};

class FotaParams {
public:
	FotaParams(FotaParamsStore *pFotaParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TFotaParams *ptFotaParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump();

	uint32_t GetServerIp() const {
		return m_tFotaParams.nServerIp;
	}

public:
    static void StaticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tFotaParams.nSetList & nMask) == nMask;
    }

private:
	FotaParamsStore *m_pFotaParamsStore;
	struct TFotaParams m_tFotaParams;
};

#endif /* FOTAPARAMS_H_ */
