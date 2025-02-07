/**
 * @file midiparams.h
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "configstore.h"

namespace midiparams {
struct Params {
	uint32_t nSetList;
	uint32_t nBaudrate;
} __attribute__((packed));

static_assert(sizeof(struct Params) <= 32, "struct Params is too large");

struct Mask {
	static constexpr uint32_t BAUDRATE = (1U << 0);
	static constexpr uint32_t ACTIVE_SENSE = (1U << 1);
};
}  // namespace midiparams

class MidiParamsStore {
public:
	static void Update(const struct midiparams::Params *pMidiParams) {
		ConfigStore::Get()->Update(configstore::Store::MIDI, pMidiParams, sizeof(struct midiparams::Params));
	}

	static void Copy(struct midiparams::Params *pMidiParams) {
		ConfigStore::Get()->Copy(configstore::Store::MIDI, pMidiParams, sizeof(struct midiparams::Params));
	}
};

class MidiParams {
public:
	MidiParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const midiparams::Params *ptMidiParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set();

	uint32_t GetBaudrate() const {
		return m_Params.nBaudrate;
	}

	bool GetActiveSense() const {
		return isMaskSet(midiparams::Mask::ACTIVE_SENSE);
	}

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *pLine);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    midiparams::Params m_Params;
};

#endif /* MIDIPARAMS_H_ */
