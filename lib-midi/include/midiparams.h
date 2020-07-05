/**
 * @file midiparams.h
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

#ifndef MIDIPARAMS_H_
#define MIDIPARAMS_H_

#include <stdint.h>

struct TMidiParams {
	uint32_t nSetList;
	uint32_t nBaudrate;
	uint8_t nActiveSense;
} __attribute__((packed));

struct MidiParamsMask {
	static constexpr auto BAUDRATE = (1U << 0);
	static constexpr auto ACTIVE_SENSE = (1U << 1);
};

class MidiParamsStore {
public:
	virtual ~MidiParamsStore() {
	}

	virtual void Update(const struct TMidiParams *pMidiParams)=0;
	virtual void Copy(struct TMidiParams *pMidiParams)=0;
};

class MidiParams {
public:
	MidiParams(MidiParamsStore *pMidiParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TMidiParams *ptMidiParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Set();

	void Dump();

	uint32_t GetBaudrate() {
		return m_tMidiParams.nBaudrate;
	}

	bool GetActiveSense() {
		return (m_tMidiParams.nActiveSense != 0);
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
    bool isMaskSet(uint32_t nMask) {
    	return (m_tMidiParams.nSetList & nMask) == nMask;
    }

private:
    MidiParamsStore *m_pMidiParamsStore;
    struct TMidiParams m_tMidiParams;
};

#endif /* MIDIPARAMS_H_ */
