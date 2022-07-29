/**
 * @file showfileparams.cpp
 *
 */
/* Copyright (C) 2020-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
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
#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
# include "e131controller.h"
# include "artnetcontroller.h"
#endif

#ifndef DMX_MAX_VALUE
# define DMX_MAX_VALUE 255
#endif

#include "debug.h"

struct PROTOCOL2STRING {
	static const char *Get(uint8_t p) {
		return (p == static_cast<uint8_t>(showfile::Protocols::SACN)) ? "sACN" : "Art-Net";
	}
};

ShowFileParams::ShowFileParams(ShowFileParamsStore *pShowFileParamsStore): m_pShowFileParamsStore(pShowFileParamsStore) {
	DEBUG_ENTRY

	m_showFileParams.nSetList = 0;
	m_showFileParams.nFormat = static_cast<uint8_t>(showfile::Formats::OLA);
	m_showFileParams.nShow = 0;
	m_showFileParams.nOptions = 0;
	m_showFileParams.nOscPortIncoming = osc::port::DEFAULT_INCOMING;
	m_showFileParams.nOscPortOutgoing = osc::port::DEFAULT_OUTGOING;
#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
	m_showFileParams.nProtocol = static_cast<uint8_t>(showfile::Protocols::SACN);
	m_showFileParams.nUniverse = DEFAULT_SYNCHRONIZATION_ADDRESS;
#else
	m_showFileParams.nProtocol = static_cast<uint8_t>(showfile::Protocols::INTERNAL);
#endif
	m_showFileParams.nDisableUnicast = 0;
	m_showFileParams.nDmxMaster = DMX_MAX_VALUE;

	DEBUG_EXIT
}

bool ShowFileParams::Load() {
	m_showFileParams.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(ShowFileParams::staticCallbackFunction, this);

	if (configfile.Read(ShowFileParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pShowFileParamsStore != nullptr) {
			m_pShowFileParamsStore->Update(&m_showFileParams);
		}
	} else
#endif
	if (m_pShowFileParamsStore != nullptr) {
		m_pShowFileParamsStore->Copy(&m_showFileParams);
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

	m_showFileParams.nSetList = 0;

	ReadConfigFile config(ShowFileParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pShowFileParamsStore->Update(&m_showFileParams);
}

void ShowFileParams::HandleOptions(const char *pLine, const char *pKeyword, uint16_t nMask) {
	uint8_t value8;

	if (Sscan::Uint8(pLine, pKeyword, value8) == Sscan::OK) {
		if (value8 != 0) {
			m_showFileParams.nOptions |= nMask;
			m_showFileParams.nSetList |= showfileparams::Mask::OPTIONS;
		} else {
			m_showFileParams.nOptions &= static_cast<uint16_t>(~nMask);
		}

		if (m_showFileParams.nOptions == 0) {
			m_showFileParams.nSetList &= ~showfileparams::Mask::OPTIONS;
		}
	}
}

void ShowFileParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char aValue[16];
	uint32_t nLength = ShowFileConst::SHOWFILECONST_FORMAT_NAME_LENGTH - 1;

	if (Sscan::Char(pLine, ShowFileParamsConst::FORMAT, aValue, nLength) == Sscan::OK) {
		aValue[nLength] = '\0';

		showfile::Formats tFormat = ShowFile::GetFormat(aValue);

		if (tFormat != showfile::Formats::UNDEFINED) {
			m_showFileParams.nFormat = static_cast<uint8_t>(tFormat);
			m_showFileParams.nSetList |= showfileparams::Mask::FORMAT;
		} else {
			m_showFileParams.nFormat = static_cast<uint8_t>(showfile::Formats::OLA);
			m_showFileParams.nSetList &= ~showfileparams::Mask::FORMAT;
		}

		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, OscParamsConst::INCOMING_PORT, nValue16) == Sscan::OK) {
		if ((nValue16 != osc::port::DEFAULT_INCOMING) && (nValue16 > 1023)) {
			m_showFileParams.nOscPortIncoming = nValue16;
			m_showFileParams.nSetList |= showfileparams::Mask::OSC_PORT_INCOMING;
		} else {
			m_showFileParams.nOscPortIncoming = osc::port::DEFAULT_INCOMING;
			m_showFileParams.nSetList &= ~showfileparams::Mask::OSC_PORT_INCOMING;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::OUTGOING_PORT, nValue16) == Sscan::OK) {
		if ((nValue16 != osc::port::DEFAULT_OUTGOING) && (nValue16 > 1023)) {
			m_showFileParams.nOscPortOutgoing = nValue16;
			m_showFileParams.nSetList |= showfileparams::Mask::OSC_PORT_OUTGOING;
		} else {
			m_showFileParams.nOscPortOutgoing = osc::port::DEFAULT_OUTGOING;
			m_showFileParams.nSetList &= ~showfileparams::Mask::OSC_PORT_OUTGOING;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, ShowFileParamsConst::SHOW, nValue8) == Sscan::OK) {
		if (nValue8 < showfile::File::MAX_NUMBER) {
			m_showFileParams.nShow = nValue8;
			m_showFileParams.nSetList |= showfileparams::Mask::SHOW;
		} else {
			m_showFileParams.nShow = 0;
			m_showFileParams.nSetList &= ~showfileparams::Mask::SHOW;
		}
		return;
	}

#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
	nLength = 6;

	if (Sscan::Char(pLine, ShowFileParamsConst::PROTOCOL, aValue, nLength) == Sscan::OK) {
		aValue[nLength] = '\0';

		if(strcasecmp(aValue, "artnet") == 0) {
			m_showFileParams.nProtocol = static_cast<uint8_t>(showfile::Protocols::ARTNET);
			m_showFileParams.nSetList |= showfileparams::Mask::PROTOCOL;
		} else {
			m_showFileParams.nProtocol = static_cast<uint8_t>(showfile::Protocols::SACN);
			m_showFileParams.nSetList &= showfileparams::Mask::PROTOCOL;
		}
		return;
	}

	if (Sscan::Uint16(pLine, ShowFileParamsConst::SACN_SYNC_UNIVERSE, nValue16) == Sscan::OK) {
		if (nValue16 > e131::universe::MAX) {
			m_showFileParams.nUniverse = DEFAULT_SYNCHRONIZATION_ADDRESS;
			m_showFileParams.nSetList &= ~showfileparams::Mask::SACN_UNIVERSE;
		} else {
			m_showFileParams.nUniverse = nValue16;
			m_showFileParams.nSetList |= showfileparams::Mask::SACN_UNIVERSE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ShowFileParamsConst::ARTNET_DISABLE_UNICAST, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_showFileParams.nDisableUnicast = 1;
			m_showFileParams.nSetList |= showfileparams::Mask::ARTNET_UNICAST_DISABLED;
		} else {
			m_showFileParams.nDisableUnicast = 0;
			m_showFileParams.nSetList &= ~showfileparams::Mask::ARTNET_UNICAST_DISABLED;
		}
		return;
	}
#endif

	if (Sscan::Uint8(pLine, ShowFileParamsConst::DMX_MASTER, nValue8) == Sscan::OK) {
		if (nValue8 < DMX_MAX_VALUE) {
			m_showFileParams.nDmxMaster = nValue8;
			m_showFileParams.nSetList |= showfileparams::Mask::DMX_MASTER;
		} else {
			m_showFileParams.nDmxMaster = DMX_MAX_VALUE;
			m_showFileParams.nSetList &= ~showfileparams::Mask::DMX_MASTER;
		}
		return;
	}

	HandleOptions(pLine, ShowFileParamsConst::OPTION_AUTO_START, showfileparams::Options::AUTO_START);
	HandleOptions(pLine, ShowFileParamsConst::OPTION_LOOP, showfileparams::Options::LOOP);
	HandleOptions(pLine, ShowFileParamsConst::OPTION_DISABLE_SYNC, showfileparams::Options::DISABLE_SYNC);
}

void ShowFileParams::Builder(const struct TShowFileParams *ptShowFileParamss, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (ptShowFileParamss != nullptr) {
		memcpy(&m_showFileParams, ptShowFileParamss, sizeof(struct showfileparams::Params));
	} else {
		m_pShowFileParamsStore->Copy(&m_showFileParams);
	}

	PropertiesBuilder builder(ShowFileParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(ShowFileParamsConst::SHOW, static_cast<uint32_t>(m_showFileParams.nShow), isMaskSet(showfileparams::Mask::SHOW));

	builder.Add(ShowFileParamsConst::FORMAT, ShowFile::GetFormat(static_cast<showfile::Formats>(m_showFileParams.nFormat)), isMaskSet(showfileparams::Mask::FORMAT));
#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
	builder.Add(ShowFileParamsConst::PROTOCOL, (m_showFileParams.nProtocol == static_cast<uint8_t>(showfile::Protocols::SACN)) ? "sacn" : "artnet", isMaskSet(showfileparams::Mask::PROTOCOL));
#endif

	builder.AddComment("DMX");
	builder.Add(ShowFileParamsConst::DMX_MASTER, static_cast<uint32_t>(m_showFileParams.nDmxMaster), isMaskSet(showfileparams::Mask::DMX_MASTER));

#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
	builder.AddComment("sACN");
	builder.Add(ShowFileParamsConst::SACN_SYNC_UNIVERSE, static_cast<uint32_t>(m_showFileParams.nUniverse), isMaskSet(showfileparams::Mask::SACN_UNIVERSE));

	builder.AddComment("Art-Net");
	builder.Add(ShowFileParamsConst::ARTNET_DISABLE_UNICAST, static_cast<uint32_t>(m_showFileParams.nDisableUnicast), isMaskSet(showfileparams::Mask::ARTNET_UNICAST_DISABLED));
#endif

	builder.AddComment("Options");
	builder.Add(ShowFileParamsConst::OPTION_AUTO_START, isOptionSet(showfileparams::Options::AUTO_START), isOptionSet(showfileparams::Options::AUTO_START));
	builder.Add(ShowFileParamsConst::OPTION_LOOP, isOptionSet(showfileparams::Options::LOOP), isOptionSet(showfileparams::Options::LOOP));
#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
	builder.Add(ShowFileParamsConst::OPTION_DISABLE_SYNC, isOptionSet(showfileparams::Options::DISABLE_SYNC), isOptionSet(showfileparams::Options::DISABLE_SYNC));
#endif

	builder.AddComment("OSC Server");
	builder.Add(OscParamsConst::INCOMING_PORT, static_cast<uint32_t>(m_showFileParams.nOscPortIncoming), isMaskSet(showfileparams::Mask::OSC_PORT_INCOMING));
	builder.Add(OscParamsConst::OUTGOING_PORT, static_cast<uint32_t>(m_showFileParams.nOscPortOutgoing), isMaskSet(showfileparams::Mask::OSC_PORT_OUTGOING));

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void ShowFileParams::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
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

	if (isMaskSet(showfileparams::Mask::OSC_PORT_INCOMING)) {
		ShowFileOSC::Get()->SetPortIncoming(m_showFileParams.nOscPortIncoming);
	}

	if (isMaskSet(showfileparams::Mask::OSC_PORT_OUTGOING)) {
		ShowFileOSC::Get()->SetPortOutgoing(m_showFileParams.nOscPortOutgoing);
	}

#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
	// sACN E1.31

	if (isMaskSet(showfileparams::Mask::SACN_UNIVERSE)) {
		if (E131Controller::Get() != nullptr) {
			E131Controller::Get()->SetSynchronizationAddress(m_showFileParams.nUniverse);
		}
	}

	// Art-Net

	if (isMaskSet(showfileparams::Mask::ARTNET_UNICAST_DISABLED)) {
		if (ArtNetController::Get() != nullptr) {
			ArtNetController::Get()->SetUnicast(false);
		}
	}
#endif

	// Options

	if (isOptionSet(showfileparams::Options::LOOP)) {
		ShowFile::Get()->DoLoop(true);
	}

#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
	if (isOptionSet(showfileparams::Options::DISABLE_SYNC)) {
		if (E131Controller::Get() != nullptr) {
			E131Controller::Get()->SetSynchronizationAddress(0);
		}

		if (ArtNetController::Get() != nullptr) {
			ArtNetController::Get()->SetSynchronization(false);
		}
	}
#endif

	DEBUG_EXIT
}

void ShowFileParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, ShowFileParamsConst::FILE_NAME);

	if(isMaskSet(showfileparams::Mask::FORMAT)) {
		printf(" %s=%d [%s]\n", ShowFileParamsConst::FORMAT, m_showFileParams.nFormat, ShowFile::GetFormat(static_cast<showfile::Formats>(m_showFileParams.nFormat)));
	}

	if (isMaskSet(showfileparams::Mask::SHOW)) {
		printf(" %s=%u\n", ShowFileParamsConst::SHOW, m_showFileParams.nShow);
	}

	if (isMaskSet(showfileparams::Mask::DMX_MASTER)) {
		printf(" %s=%u\n", ShowFileParamsConst::DMX_MASTER, m_showFileParams.nDmxMaster);
	}

#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
	if(isMaskSet(showfileparams::Mask::PROTOCOL)) {
		printf(" %s=%d [%s]\n", ShowFileParamsConst::PROTOCOL, m_showFileParams.nProtocol, PROTOCOL2STRING::Get(m_showFileParams.nProtocol));
	}

	if (isMaskSet(showfileparams::Mask::SACN_UNIVERSE)) {
		printf(" %s=%u\n", ShowFileParamsConst::SACN_SYNC_UNIVERSE, m_showFileParams.nUniverse);
	}

	if (isMaskSet(showfileparams::Mask::ARTNET_UNICAST_DISABLED)) {
		printf(" %s=%u [%s]\n", ShowFileParamsConst::ARTNET_DISABLE_UNICAST, m_showFileParams.nDisableUnicast, m_showFileParams.nDisableUnicast == 0 ? "No" : "Yes");
	}
#endif

	// Options

	if (isMaskSet(showfileparams::Mask::OPTIONS)) {
		printf(" Options 0x%.2x\n", m_showFileParams.nOptions);

		if (isOptionSet(showfileparams::Options::AUTO_START)) {
			printf("  Auto start is enabled\n");
		}

		if (isOptionSet(showfileparams::Options::LOOP)) {
			printf("  Loop is enabled\n");
		}
#if !defined (CONFIG_SHOWFILE_LIGHTSET_ONLY)
		if (isOptionSet(showfileparams::Options::DISABLE_SYNC)) {
			printf("  Synchronization is disabled\n");
		}
#endif
	}

	if (isMaskSet(showfileparams::Mask::OSC_PORT_INCOMING)) {
		printf(" %s=%u\n", OscParamsConst::INCOMING_PORT, m_showFileParams.nOscPortIncoming);
	}

	if (isMaskSet(showfileparams::Mask::OSC_PORT_OUTGOING)) {
		printf(" %s=%u\n", OscParamsConst::OUTGOING_PORT, m_showFileParams.nOscPortOutgoing);
	}
#endif
}

void ShowFileParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ShowFileParams *>(p))->callbackFunction(s);
}
