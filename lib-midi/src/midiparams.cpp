/**
 * @file midiparams.cpp
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

MidiParams::MidiParams() {
	m_Params.nSetList = midiparams::Mask::ACTIVE_SENSE;
	m_Params.nBaudrate = defaults::BAUDRATE;
}

void MidiParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = midiparams::Mask::ACTIVE_SENSE;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(MidiParams::StaticCallbackFunction, this);

	if (configfile.Read(MidiParamsConst::FILE_NAME)) {
		MidiParamsStore::Update(&m_Params);
	} else
#endif
	MidiParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void MidiParams::Load(const char* pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = midiparams::Mask::ACTIVE_SENSE;

	ReadConfigFile config(MidiParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	MidiParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void MidiParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint32_t nValue32;

	if (Sscan::Uint32(pLine, MidiParamsConst::BAUDRATE, nValue32) == Sscan::OK) {
		if ((nValue32 != defaults::BAUDRATE) && (nValue32 >= 9600) && (nValue32 <= 115200)) {
			m_Params.nBaudrate = nValue32;
			m_Params.nSetList |= midiparams::Mask::BAUDRATE;
		} else {
			m_Params.nBaudrate = defaults::BAUDRATE;
			m_Params.nSetList &= ~midiparams::Mask::BAUDRATE;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, MidiParamsConst::ACTIVE_SENSE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= midiparams::Mask::ACTIVE_SENSE;
		} else {
			m_Params.nSetList &= ~midiparams::Mask::ACTIVE_SENSE;
		}
		return;
	}
}

void MidiParams::Set() {
	if (isMaskSet(midiparams::Mask::BAUDRATE)) {
		Midi::Get()->SetBaudrate(m_Params.nBaudrate);
	}

	if (isMaskSet(midiparams::Mask::ACTIVE_SENSE)) {
		Midi::Get()->SetActiveSense(true);
	}
}

void MidiParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<MidiParams*>(p))->callbackFunction(s);
}

void MidiParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, MidiParamsConst::FILE_NAME);

	if (isMaskSet(midiparams::Mask::BAUDRATE)) {
		printf(" %s=%d\n", MidiParamsConst::BAUDRATE, m_Params.nBaudrate);
	}

	if (isMaskSet(midiparams::Mask::ACTIVE_SENSE)) {
		printf(" %s=1 [Yes]\n", MidiParamsConst::ACTIVE_SENSE);
	}
}
