/**
 * @file showfileparams.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#include "showfileparams.h"
#include "showfileparamsconst.h"
#include "showfile.h"
#include "showfileconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "showfileosc.h"
#include "oscparamsconst.h"

// Protocols
#include "e131controller.h"
#include "artnetcontroller.h"

#include "debug.h"

struct PROTOCOL2STRING {
	static const char *Get(uint8_t p) {
		return (p == static_cast<uint8_t>(ShowFileProtocols::SACN)) ? "sACN" : "Art-Net";
	}
};

ShowFileParams::ShowFileParams(ShowFileParamsStore *pShowFileParamsStore): m_pShowFileParamsStore(pShowFileParamsStore) {
	DEBUG_ENTRY

	m_tShowFileParams.nSetList = 0;
	m_tShowFileParams.nFormat = static_cast<uint8_t>(ShowFileFormats::OLA);
	m_tShowFileParams.nShow = 0;
	m_tShowFileParams.nOptions = 0;
	m_tShowFileParams.nOscPortIncoming = osc::port::DEFAULT_INCOMING;
	m_tShowFileParams.nOscPortOutgoing = osc::port::DEFAULT_OUTGOING;
	m_tShowFileParams.nProtocol = static_cast<uint8_t>(ShowFileProtocols::SACN);
	m_tShowFileParams.nUniverse = DEFAULT_SYNCHRONIZATION_ADDRESS;
	m_tShowFileParams.nDisableUnicast = 0;
	m_tShowFileParams.nDmxMaster = DMX_MAX_VALUE;

	DEBUG_EXIT
}

bool ShowFileParams::Load() {
	m_tShowFileParams.nSetList = 0;

	ReadConfigFile configfile(ShowFileParams::staticCallbackFunction, this);

	if (configfile.Read(ShowFileParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pShowFileParamsStore != nullptr) {
			m_pShowFileParamsStore->Update(&m_tShowFileParams);
		}
	} else if (m_pShowFileParamsStore != nullptr) {
		m_pShowFileParamsStore->Copy(&m_tShowFileParams);
	} else {
		return false;
	}

	return true;
}

void ShowFileParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pShowFileParamsStore != nullptr);

	if (m_pShowFileParamsStore == nullptr) {
		return;
	}

	m_tShowFileParams.nSetList = 0;

	ReadConfigFile config(ShowFileParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pShowFileParamsStore->Update(&m_tShowFileParams);
}

void ShowFileParams::HandleOptions(const char *pLine, const char *pKeyword, uint16_t nMask) {
	uint8_t value8;

	if (Sscan::Uint8(pLine, pKeyword, value8) == Sscan::OK) {
		if (value8 != 0) {
			m_tShowFileParams.nOptions |= nMask;
			m_tShowFileParams.nSetList |= ShowFileParamsMask::OPTIONS;
		} else {
			m_tShowFileParams.nOptions &= ~nMask;
		}

		if (m_tShowFileParams.nOptions == 0) {
			m_tShowFileParams.nSetList &= ~ShowFileParamsMask::OPTIONS;
		}
	}
}

void ShowFileParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char aValue[16];
	uint32_t nLength;
	uint8_t nValue8;
	uint16_t nValue16;

	nLength = ShowFileConst::SHOWFILECONST_FORMAT_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ShowFileParamsConst::FORMAT, aValue, nLength) == Sscan::OK) {
		aValue[nLength] = '\0';

		ShowFileFormats tFormat = ShowFile::GetFormat(aValue);

		if (tFormat != ShowFileFormats::UNDEFINED) {
			m_tShowFileParams.nFormat = static_cast<uint8_t>(tFormat);
			m_tShowFileParams.nSetList |= ShowFileParamsMask::FORMAT;
		} else {
			m_tShowFileParams.nFormat = static_cast<uint8_t>(ShowFileFormats::OLA);
			m_tShowFileParams.nSetList &= ~ShowFileParamsMask::FORMAT;
		}

		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::INCOMING_PORT, nValue16) == Sscan::OK) {
		if ((nValue16 != osc::port::DEFAULT_INCOMING) && (nValue16 > 1023)) {
			m_tShowFileParams.nOscPortIncoming = nValue16;
			m_tShowFileParams.nSetList |= ShowFileParamsMask::OSC_PORT_INCOMING;
		} else {
			m_tShowFileParams.nOscPortIncoming = osc::port::DEFAULT_INCOMING;
			m_tShowFileParams.nSetList &= ~ShowFileParamsMask::OSC_PORT_INCOMING;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::OUTGOING_PORT, nValue16) == Sscan::OK) {
		if ((nValue16 != osc::port::DEFAULT_OUTGOING) && (nValue16 > 1023)) {
			m_tShowFileParams.nOscPortOutgoing = nValue16;
			m_tShowFileParams.nSetList |= ShowFileParamsMask::OSC_PORT_OUTGOING;
		} else {
			m_tShowFileParams.nOscPortOutgoing = osc::port::DEFAULT_OUTGOING;
			m_tShowFileParams.nSetList &= ~ShowFileParamsMask::OSC_PORT_OUTGOING;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ShowFileParamsConst::SHOW, nValue8) == Sscan::OK) {
		if (nValue8 < ShowFileFile::MAX_NUMBER) {
			m_tShowFileParams.nShow = nValue8;
			m_tShowFileParams.nSetList |= ShowFileParamsMask::SHOW;
		} else {
			m_tShowFileParams.nShow = 0;
			m_tShowFileParams.nSetList &= ~ShowFileParamsMask::SHOW;
		}
		return;
	}

	nLength = 6;
	if (Sscan::Char(pLine, ShowFileParamsConst::PROTOCOL, aValue, nLength) == Sscan::OK) {
		aValue[nLength] = '\0';

		if(strcasecmp(aValue, "artnet") == 0) {
			m_tShowFileParams.nProtocol = static_cast<uint8_t>(ShowFileProtocols::ARTNET);
			m_tShowFileParams.nSetList |= ShowFileParamsMask::PROTOCOL;
		} else {
			m_tShowFileParams.nProtocol = static_cast<uint8_t>(ShowFileProtocols::SACN);
			m_tShowFileParams.nSetList &= ShowFileParamsMask::PROTOCOL;
		}
		return;
	}

	if (Sscan::Uint16(pLine, ShowFileParamsConst::SACN_SYNC_UNIVERSE, nValue16) == Sscan::OK) {
		if (nValue16 > E131_UNIVERSE_MAX) {
			m_tShowFileParams.nUniverse = DEFAULT_SYNCHRONIZATION_ADDRESS;
			m_tShowFileParams.nSetList &= ~ShowFileParamsMask::SACN_UNIVERSE;
		} else {
			m_tShowFileParams.nUniverse = nValue16;
			m_tShowFileParams.nSetList |= ShowFileParamsMask::SACN_UNIVERSE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ShowFileParamsConst::ARTNET_DISABLE_UNICAST, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tShowFileParams.nDisableUnicast = 1;
			m_tShowFileParams.nSetList |= ShowFileParamsMask::ARTNET_UNICAST_DISABLED;
		} else {
			m_tShowFileParams.nDisableUnicast = 0;
			m_tShowFileParams.nSetList &= ~ShowFileParamsMask::ARTNET_UNICAST_DISABLED;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ShowFileParamsConst::DMX_MASTER, nValue8) == Sscan::OK) {
		if (nValue8 < DMX_MAX_VALUE) {
			m_tShowFileParams.nDmxMaster = nValue8;
			m_tShowFileParams.nSetList |= ShowFileParamsMask::DMX_MASTER;
		} else {
			m_tShowFileParams.nDmxMaster = DMX_MAX_VALUE;
			m_tShowFileParams.nSetList &= ~ShowFileParamsMask::DMX_MASTER;
		}
		return;
	}

	HandleOptions(pLine, ShowFileParamsConst::OPTION_AUTO_START, ShowFileOptions::AUTO_START);
	HandleOptions(pLine, ShowFileParamsConst::OPTION_LOOP, ShowFileOptions::LOOP);
	HandleOptions(pLine, ShowFileParamsConst::OPTION_DISABLE_SYNC, ShowFileOptions::DISABLE_SYNC);
}

void ShowFileParams::Builder(const struct TShowFileParams *ptShowFileParamss, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (ptShowFileParamss != nullptr) {
		memcpy(&m_tShowFileParams, ptShowFileParamss, sizeof(struct TShowFileParams));
	} else {
		m_pShowFileParamsStore->Copy(&m_tShowFileParams);
	}

	PropertiesBuilder builder(ShowFileParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(ShowFileParamsConst::SHOW, static_cast<uint32_t>(m_tShowFileParams.nShow), isMaskSet(ShowFileParamsMask::SHOW));

	builder.Add(ShowFileParamsConst::FORMAT, ShowFile::GetFormat(static_cast<ShowFileFormats>(m_tShowFileParams.nFormat)), isMaskSet(ShowFileParamsMask::FORMAT));
	builder.Add(ShowFileParamsConst::PROTOCOL, (m_tShowFileParams.nProtocol == static_cast<uint8_t>(ShowFileProtocols::SACN)) ? "sacn" : "artnet", isMaskSet(ShowFileParamsMask::PROTOCOL));

	builder.AddComment("DMX");
	builder.Add(ShowFileParamsConst::DMX_MASTER, static_cast<uint32_t>(m_tShowFileParams.nDmxMaster), isMaskSet(ShowFileParamsMask::DMX_MASTER));

	builder.AddComment("sACN");
	builder.Add(ShowFileParamsConst::SACN_SYNC_UNIVERSE, static_cast<uint32_t>(m_tShowFileParams.nUniverse), isMaskSet(ShowFileParamsMask::SACN_UNIVERSE));

	builder.AddComment("Art-Net");
	builder.Add(ShowFileParamsConst::ARTNET_DISABLE_UNICAST, static_cast<uint32_t>(m_tShowFileParams.nDisableUnicast), isMaskSet(ShowFileParamsMask::ARTNET_UNICAST_DISABLED));

	builder.AddComment("Options");
	builder.Add(ShowFileParamsConst::OPTION_AUTO_START, isOptionSet(ShowFileOptions::AUTO_START), isOptionSet(ShowFileOptions::AUTO_START));
	builder.Add(ShowFileParamsConst::OPTION_LOOP, isOptionSet(ShowFileOptions::LOOP), isOptionSet(ShowFileOptions::LOOP));
	builder.Add(ShowFileParamsConst::OPTION_DISABLE_SYNC, isOptionSet(ShowFileOptions::DISABLE_SYNC), isOptionSet(ShowFileOptions::DISABLE_SYNC));

	builder.AddComment("OSC Server");
	builder.Add(OscParamsConst::INCOMING_PORT, static_cast<uint32_t>(m_tShowFileParams.nOscPortIncoming), isMaskSet(ShowFileParamsMask::OSC_PORT_INCOMING));
	builder.Add(OscParamsConst::OUTGOING_PORT, static_cast<uint32_t>(m_tShowFileParams.nOscPortOutgoing), isMaskSet(ShowFileParamsMask::OSC_PORT_OUTGOING));

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void ShowFileParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pShowFileParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);

	DEBUG_EXIT
	return;
}

void ShowFileParams::Set() {
	DEBUG_ENTRY

	if (isMaskSet(ShowFileParamsMask::OSC_PORT_INCOMING)) {
		ShowFileOSC::Get()->SetPortIncoming(m_tShowFileParams.nOscPortIncoming);
	}

	if (isMaskSet(ShowFileParamsMask::OSC_PORT_OUTGOING)) {
		ShowFileOSC::Get()->SetPortOutgoing(m_tShowFileParams.nOscPortOutgoing);
	}

	// sACN E1.31

	if (isMaskSet(ShowFileParamsMask::SACN_UNIVERSE)) {
		if (E131Controller::Get() != nullptr) {
			E131Controller::Get()->SetSynchronizationAddress(m_tShowFileParams.nUniverse);
		}
	}

	// Art-Net

	if (isMaskSet(ShowFileParamsMask::ARTNET_UNICAST_DISABLED)) {
		if (ArtNetController::Get() != nullptr) {
			ArtNetController::Get()->SetUnicast(false);
		}
	}

	// Options

	if (isOptionSet(ShowFileOptions::LOOP)) {
		ShowFile::Get()->DoLoop(true);
	}

	if (isOptionSet(ShowFileOptions::DISABLE_SYNC)) {
		if (E131Controller::Get() != nullptr) {
			E131Controller::Get()->SetSynchronizationAddress(0);
		}

		if (ArtNetController::Get() != nullptr) {
			ArtNetController::Get()->SetSynchronization(false);
		}
	}

	DEBUG_EXIT
}

void ShowFileParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, ShowFileParamsConst::FILE_NAME);

	if(isMaskSet(ShowFileParamsMask::FORMAT)) {
		printf(" %s=%d [%s]\n", ShowFileParamsConst::FORMAT, m_tShowFileParams.nFormat, ShowFile::GetFormat(static_cast<ShowFileFormats>(m_tShowFileParams.nFormat)));
	}

	if (isMaskSet(ShowFileParamsMask::SHOW)) {
		printf(" %s=%u\n", ShowFileParamsConst::SHOW, m_tShowFileParams.nShow);
	}

	if(isMaskSet(ShowFileParamsMask::PROTOCOL)) {
		printf(" %s=%d [%s]\n", ShowFileParamsConst::PROTOCOL, m_tShowFileParams.nProtocol, PROTOCOL2STRING::Get(m_tShowFileParams.nProtocol));
	}

	if (isMaskSet(ShowFileParamsMask::DMX_MASTER)) {
		printf(" %s=%u\n", ShowFileParamsConst::DMX_MASTER, m_tShowFileParams.nDmxMaster);
	}

	if (isMaskSet(ShowFileParamsMask::SACN_UNIVERSE)) {
		printf(" %s=%u\n", ShowFileParamsConst::SACN_SYNC_UNIVERSE, m_tShowFileParams.nUniverse);
	}

	if (isMaskSet(ShowFileParamsMask::ARTNET_UNICAST_DISABLED)) {
		printf(" %s=%u [%s]\n", ShowFileParamsConst::ARTNET_DISABLE_UNICAST, m_tShowFileParams.nDisableUnicast, m_tShowFileParams.nDisableUnicast == 0 ? "No" : "Yes");
	}

	// Options

	if (isMaskSet(ShowFileParamsMask::OPTIONS)) {
		printf(" Options 0x%.2x\n", m_tShowFileParams.nOptions);

		if (isOptionSet(ShowFileOptions::AUTO_START)) {
			printf("  Auto start is enabled\n");
		}
		if (isOptionSet(ShowFileOptions::LOOP)) {
			printf("  Loop is enabled\n");
		}
		if (isOptionSet(ShowFileOptions::DISABLE_SYNC)) {
			printf("  Synchronization is disabled\n");
		}
	}

	if (isMaskSet(ShowFileParamsMask::OSC_PORT_INCOMING)) {
		printf(" %s=%u\n", OscParamsConst::INCOMING_PORT, m_tShowFileParams.nOscPortIncoming);
	}

	if (isMaskSet(ShowFileParamsMask::OSC_PORT_OUTGOING)) {
		printf(" %s=%u\n", OscParamsConst::OUTGOING_PORT, m_tShowFileParams.nOscPortOutgoing);
	}
#endif
}

void ShowFileParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ShowFileParams *>(p))->callbackFunction(s);
}
