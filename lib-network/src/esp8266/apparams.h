/**
 * @file apparams.h
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

#ifndef APPARAMS_H_
#define APPARAMS_H_

#include <cstdint>

struct TApParams {
	uint32_t nSetList;
	char aPassword[34];
}__attribute__((packed));

static_assert(sizeof(struct TApParams) <= 48, "struct TApParams is too large");

struct ApParamsMask {
	static constexpr auto PASSWORD = (1U << 0);
};

class ApParamsStore {
public:
	virtual ~ApParamsStore() {
	}

	virtual void Update(const struct TApParams *pApParams)=0;
	virtual void Copy(struct TApParams *pApParams)=0;
};

class ApParams {
public:
	ApParams(ApParamsStore *pApParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TApParams *ptApParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump();

	const char *GetPassword() const {
		return m_tApParams.aPassword;
	}

public:
    static void StaticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tApParams.nSetList & nMask) == nMask;
    }

private:
	ApParamsStore *m_pApParamsStore;
	struct TApParams m_tApParams;
};

#endif /* APPARAMS_H_ */
