/**
 * @file midiparams.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cassert>

#include "midiparams.h"
#include "midiparamsconst.h"

#include "midi.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

MidiParams::MidiParams(MidiParamsStore* pMidiParamsStore): m_pMidiParamsStore(pMidiParamsStore) {
	memset(&m_tMidiParams, 0, sizeof(struct TMidiParams));
	m_tMidiParams.nBaudrate = MIDI_BAUDRATE_DEFAULT;
	m_tMidiParams.nActiveSense = 1;
}

bool MidiParams::Load() {
	m_tMidiParams.nSetList = 0;

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

	m_tMidiParams.nSetList = 0;

	ReadConfigFile config(MidiParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pMidiParamsStore->Update(&m_tMidiParams);
}

void MidiParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	uint32_t nValue32;

	if (Sscan::Uint32(pLine, MidiParamsConst::BAUDRATE, nValue32) == Sscan::OK) {
		if ((nValue32 != MIDI_BAUDRATE_DEFAULT) && (nValue32 >= 9600) && (nValue32 <= 115200)) {
			m_tMidiParams.nBaudrate = nValue32;
			m_tMidiParams.nSetList |= MidiParamsMask::BAUDRATE;
		} else {
			m_tMidiParams.nBaudrate = MIDI_BAUDRATE_DEFAULT;
			m_tMidiParams.nSetList &= ~MidiParamsMask::BAUDRATE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, MidiParamsConst::ACTIVE_SENSE, nValue8) == Sscan::OK) {
		if (nValue8 == 0) {
			m_tMidiParams.nActiveSense = 0;
			m_tMidiParams.nSetList |= MidiParamsMask::ACTIVE_SENSE;
		} else {
			m_tMidiParams.nActiveSense = 1;
			m_tMidiParams.nSetList &= ~MidiParamsMask::ACTIVE_SENSE;
		}
		return;
	}
}

void MidiParams::Set() {
	if (m_tMidiParams.nSetList == 0) {
		return;
	}

	if (isMaskSet(MidiParamsMask::BAUDRATE)) {
		Midi::Get()->SetBaudrate(m_tMidiParams.nBaudrate);
	}

	if (isMaskSet(MidiParamsMask::ACTIVE_SENSE)) {
		Midi::Get()->SetActiveSense(m_tMidiParams.nActiveSense != 0);
	}
}

void MidiParams::Dump() {
#ifndef NDEBUG
	if (m_tMidiParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, MidiParamsConst::FILE_NAME);

	if (isMaskSet(MidiParamsMask::BAUDRATE)) {
		printf(" %s=%d\n", MidiParamsConst::BAUDRATE, m_tMidiParams.nBaudrate);
	}

	if (isMaskSet(MidiParamsMask::ACTIVE_SENSE)) {
		printf(" %s=%d [%s]\n", MidiParamsConst::ACTIVE_SENSE, m_tMidiParams.nActiveSense, BOOL2STRING::Get(m_tMidiParams.nActiveSense));
	}

#endif
}

void MidiParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<MidiParams*>(p))->callbackFunction(s);
}
