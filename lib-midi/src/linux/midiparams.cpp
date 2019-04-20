/**
 * @file midiparams.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "midiparams.h"
#include "midi.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#define BOOL2STRING(b)	(b) ? "Yes" : "No"

#define SET_BAUDRATE		(1 << 0)
#define SET_ACTIVE_SENSE	(1 << 1)

static const char PARAMS_FILE_NAME[] ALIGNED = "midi.txt";
static const char PARAMS_BAUDRATE[] ALIGNED = "baudrate";
static const char PARAMS_ACTIVE_SENSE[] ALIGNED = "active_sense";

MidiParams::MidiParams(MidiParamsStore* pMidiParamsStore): m_pMidiParamsStore(pMidiParamsStore) {
	uint8_t *p = (uint8_t *) &m_tMidiParams;

	for (uint32_t i = 0; i < sizeof(struct TMidiParams); i++) {
		*p++ = 0;
	}

	m_tMidiParams.nBaudrate = MIDI_BAUDRATE_DEFAULT;
	m_tMidiParams.nActiveSense = 1;
}

MidiParams::~MidiParams(void) {
	m_tMidiParams.nSetList = 0;
}

bool MidiParams::Load(void) {
	m_tMidiParams.nSetList = 0;

	ReadConfigFile configfile(MidiParams::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pMidiParamsStore != 0) {
			m_pMidiParamsStore->Update(&m_tMidiParams);
		}
	} else if (m_pMidiParamsStore != 0) {
		m_pMidiParamsStore->Copy(&m_tMidiParams);
	} else {
		return false;
	}

	return true;
}

void MidiParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pMidiParamsStore != 0);

	if (m_pMidiParamsStore == 0) {
		return;
	}

	m_tMidiParams.nSetList = 0;

	ReadConfigFile config(MidiParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pMidiParamsStore->Update(&m_tMidiParams);
}

bool MidiParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	if (m_pMidiParamsStore == 0) {
		nSize = 0;
		return false;
	}

	m_pMidiParamsStore->Copy(&m_tMidiParams);

	PropertiesBuilder builder(PARAMS_FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(PARAMS_BAUDRATE, m_tMidiParams.nBaudrate, isMaskSet(SET_BAUDRATE));
	isAdded &= builder.Add(PARAMS_ACTIVE_SENSE, (uint32_t) m_tMidiParams.nActiveSense , isMaskSet(SET_ACTIVE_SENSE));

	nSize = builder.GetSize();

	return isAdded;
}

void MidiParams::Set(void) {
}

void MidiParams::Dump(void) {
#ifndef NDEBUG
	if (m_tMidiParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_BAUDRATE)) {
		printf(" %s=%d\n", PARAMS_BAUDRATE, m_tMidiParams.nBaudrate);
	}

	if (isMaskSet(SET_ACTIVE_SENSE)) {
		printf(" %s=%d [%s]\n", PARAMS_ACTIVE_SENSE, (int) m_tMidiParams.nActiveSense, BOOL2STRING(m_tMidiParams.nActiveSense));
	}

#endif
}

bool MidiParams::isMaskSet(uint32_t nMask) const {
	return (m_tMidiParams.nSetList & nMask) == nMask;
}

void MidiParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((MidiParams *) p)->callbackFunction(s);
}

void MidiParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint32_t value32;

	if (Sscan::Uint32(pLine, PARAMS_BAUDRATE, &value32) == SSCAN_OK) {
		if (value32 == 0) {
			m_tMidiParams.nBaudrate = MIDI_BAUDRATE_DEFAULT;
			m_tMidiParams.nSetList |= SET_BAUDRATE;
		} else if ((value32 >= 9600) && (value32 <= 115200)) {
			m_tMidiParams.nBaudrate = value32;
			m_tMidiParams.nSetList |= SET_BAUDRATE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_ACTIVE_SENSE, &value8) == SSCAN_OK) {
		m_tMidiParams.nActiveSense = !(value8 == 0);
		m_tMidiParams.nSetList |= SET_ACTIVE_SENSE;
		return;
	}
}
