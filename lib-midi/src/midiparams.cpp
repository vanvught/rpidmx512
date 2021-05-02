/**
 * @file midiparams.cpp
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "midiparams.h"
#include "midiparamsconst.h"

#include "midi.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

using namespace midi;

MidiParams::MidiParams(MidiParamsStore* pMidiParamsStore): m_pMidiParamsStore(pMidiParamsStore) {
	m_tMidiParams.nSetList = MidiParamsMask::ACTIVE_SENSE;
	m_tMidiParams.nBaudrate = defaults::BAUDRATE;
}

bool MidiParams::Load() {
	m_tMidiParams.nSetList = MidiParamsMask::ACTIVE_SENSE;

	ReadConfigFile configfile(MidiParams::staticCallbackFunction, this);

	if (configfile.Read(MidiParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pMidiParamsStore != nullptr) {
			m_pMidiParamsStore->Update(&m_tMidiParams);
		}
	} else if (m_pMidiParamsStore != nullptr) {
		m_pMidiParamsStore->Copy(&m_tMidiParams);
	} else {
		return false;
	}

	return true;
}

void MidiParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pMidiParamsStore != nullptr);

	if (m_pMidiParamsStore == nullptr) {
		return;
	}

	m_tMidiParams.nSetList = MidiParamsMask::ACTIVE_SENSE;

	ReadConfigFile config(MidiParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pMidiParamsStore->Update(&m_tMidiParams);
}

void MidiParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint32_t nValue32;

	if (Sscan::Uint32(pLine, MidiParamsConst::BAUDRATE, nValue32) == Sscan::OK) {
		if ((nValue32 != defaults::BAUDRATE) && (nValue32 >= 9600) && (nValue32 <= 115200)) {
			m_tMidiParams.nBaudrate = nValue32;
			m_tMidiParams.nSetList |= MidiParamsMask::BAUDRATE;
		} else {
			m_tMidiParams.nBaudrate = defaults::BAUDRATE;
			m_tMidiParams.nSetList &= ~MidiParamsMask::BAUDRATE;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, MidiParamsConst::ACTIVE_SENSE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tMidiParams.nSetList |= MidiParamsMask::ACTIVE_SENSE;
		} else {
			m_tMidiParams.nSetList &= ~MidiParamsMask::ACTIVE_SENSE;
		}
		return;
	}
}

void MidiParams::Set() {
	if (isMaskSet(MidiParamsMask::BAUDRATE)) {
		Midi::Get()->SetBaudrate(m_tMidiParams.nBaudrate);
	}

	if (isMaskSet(MidiParamsMask::ACTIVE_SENSE)) {
		Midi::Get()->SetActiveSense(true);
	}
}

void MidiParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, MidiParamsConst::FILE_NAME);

	if (isMaskSet(MidiParamsMask::BAUDRATE)) {
		printf(" %s=%d\n", MidiParamsConst::BAUDRATE, m_tMidiParams.nBaudrate);
	}

	if (isMaskSet(MidiParamsMask::ACTIVE_SENSE)) {
		printf(" %s=%1 [Yes]\n", MidiParamsConst::ACTIVE_SENSE);
	}

#endif
}

void MidiParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<MidiParams*>(p))->callbackFunction(s);
}
