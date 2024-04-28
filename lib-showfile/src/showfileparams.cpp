/**
 * @file showfileparams.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
# include "showfileosc.h"
# include "osc.h"
# include "oscparamsconst.h"
#endif

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

// Protocols
#if !defined (CONFIG_SHOWFILE_PROTOCOL_INTERNAL)
# if defined (CONFIG_SHOWFILE_PROTOCOL_E131)
#  include "e131controller.h"
# endif
# if defined (CONFIG_SHOWFILE_PROTOCOL_ARTNET)
#  include "artnetcontroller.h"
# endif
#endif

#include "debug.h"

ShowFileParams::ShowFileParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(m_Params));

#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
	m_Params.nOscPortIncoming = osc::port::DEFAULT_INCOMING;
	m_Params.nOscPortOutgoing = osc::port::DEFAULT_OUTGOING;
#endif
#if !defined (CONFIG_SHOWFILE_PROTOCOL_INTERNAL)
# if defined (CONFIG_SHOWFILE_PROTOCOL_E131)
	m_Params.nUniverse = DEFAULT_SYNCHRONIZATION_ADDRESS;
# else
# endif
#else
#endif
	m_Params.nDmxMaster = UINT8_MAX;

	DEBUG_EXIT
}

void ShowFileParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(ShowFileParams::staticCallbackFunction, this);

	if (configfile.Read(ShowFileParamsConst::FILE_NAME)) {
		ShowFileParamsStore::Update(&m_Params);
	} else
#endif
		ShowFileParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void ShowFileParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(ShowFileParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	ShowFileParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void ShowFileParams::SetBool(const uint8_t nValue, const uint32_t nMask) {
	if (nValue != 0) {
		m_Params.nSetList |= nMask;
	} else {
		m_Params.nSetList &= ~nMask;
	}
}

void ShowFileParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
	uint16_t nValue16;

	if (Sscan::Uint16(pLine, OscParamsConst::INCOMING_PORT, nValue16) == Sscan::OK) {
		if ((nValue16 != osc::port::DEFAULT_INCOMING) && (nValue16 > 1023)) {
			m_Params.nOscPortIncoming = nValue16;
			m_Params.nSetList |= showfileparams::Mask::OSC_PORT_INCOMING;
		} else {
			m_Params.nOscPortIncoming = osc::port::DEFAULT_INCOMING;
			m_Params.nSetList &= ~showfileparams::Mask::OSC_PORT_INCOMING;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::OUTGOING_PORT, nValue16) == Sscan::OK) {
		if ((nValue16 != osc::port::DEFAULT_OUTGOING) && (nValue16 > 1023)) {
			m_Params.nOscPortOutgoing = nValue16;
			m_Params.nSetList |= showfileparams::Mask::OSC_PORT_OUTGOING;
		} else {
			m_Params.nOscPortOutgoing = osc::port::DEFAULT_OUTGOING;
			m_Params.nSetList &= ~showfileparams::Mask::OSC_PORT_OUTGOING;
		}
		return;
	}
#endif

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, ShowFileParamsConst::SHOW, nValue8) == Sscan::OK) {
		if (nValue8 < showfile::FILE_MAX_NUMBER) {
			m_Params.nShow = nValue8;
			m_Params.nSetList |= showfileparams::Mask::SHOW;
		} else {
			m_Params.nShow = 0;
			m_Params.nSetList &= ~showfileparams::Mask::SHOW;
		}
		return;
	}

#if !defined (CONFIG_SHOWFILE_PROTOCOL_INTERNAL)
# if defined (CONFIG_SHOWFILE_PROTOCOL_E131)
	if (Sscan::Uint16(pLine, ShowFileParamsConst::SACN_SYNC_UNIVERSE, nValue16) == Sscan::OK) {
		if (nValue16 > e131::universe::MAX) {
			m_Params.nUniverse = DEFAULT_SYNCHRONIZATION_ADDRESS;
			m_Params.nSetList &= ~showfileparams::Mask::SACN_UNIVERSE;
		} else {
			m_Params.nUniverse = nValue16;
			m_Params.nSetList |= showfileparams::Mask::SACN_UNIVERSE;
		}
		return;
	}
# endif
# if defined (CONFIG_SHOWFILE_PROTOCOL_ARTNET)
	if (Sscan::Uint8(pLine, ShowFileParamsConst::ARTNET_DISABLE_UNICAST, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nDisableUnicast = 1;
			m_Params.nSetList |= showfileparams::Mask::ARTNET_UNICAST_DISABLED;
		} else {
			m_Params.nDisableUnicast = 0;
			m_Params.nSetList &= ~showfileparams::Mask::ARTNET_UNICAST_DISABLED;
		}
		return;
	}
# endif
#endif

#if defined (CONFIG_SHOWFILE_ENABLE_MASTER)
	if (Sscan::Uint8(pLine, ShowFileParamsConst::DMX_MASTER, nValue8) == Sscan::OK) {
		if (nValue8 < UINT8_MAX) {
			m_Params.nDmxMaster = nValue8;
			m_Params.nSetList |= showfileparams::Mask::DMX_MASTER;
		} else {
			m_Params.nDmxMaster = UINT8_MAX;
			m_Params.nSetList &= ~showfileparams::Mask::DMX_MASTER;
		}
		return;
	}
#endif

	if (Sscan::Uint8(pLine, ShowFileParamsConst::OPTION_AUTO_PLAY, nValue8) == Sscan::OK) {
		SetBool(nValue8, showfileparams::Mask::OPTION_AUTO_PLAY);
		return;
	}

	if (Sscan::Uint8(pLine, ShowFileParamsConst::OPTION_LOOP, nValue8) == Sscan::OK) {
		SetBool(nValue8, showfileparams::Mask::OPTION_LOOP);
		return;
	}

	if (Sscan::Uint8(pLine, ShowFileParamsConst::OPTION_DISABLE_SYNC, nValue8) == Sscan::OK) {
		SetBool(nValue8, showfileparams::Mask::OPTION_DISABLE_SYNC);
		return;
	}
}

void ShowFileParams::Builder(const struct TShowFileParams *ptShowFileParamss, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (ptShowFileParamss != nullptr) {
		memcpy(&m_Params, ptShowFileParamss, sizeof(struct showfileparams::Params));
	} else {
		ShowFileParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(ShowFileParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(ShowFileParamsConst::SHOW, static_cast<uint32_t>(m_Params.nShow), isMaskSet(showfileparams::Mask::SHOW));

#if defined (CONFIG_SHOWFILE_ENABLE_MASTER)
	builder.AddComment("Pixel");
	builder.Add(ShowFileParamsConst::DMX_MASTER, static_cast<uint32_t>(m_Params.nDmxMaster), isMaskSet(showfileparams::Mask::DMX_MASTER));
#endif

#if !defined (CONFIG_SHOWFILE_PROTOCOL_INTERNAL)
# if defined (CONFIG_SHOWFILE_PROTOCOL_E131)
	builder.AddComment("sACN");
	builder.Add(ShowFileParamsConst::SACN_SYNC_UNIVERSE, static_cast<uint32_t>(m_Params.nUniverse), isMaskSet(showfileparams::Mask::SACN_UNIVERSE));
# endif
# if defined (CONFIG_SHOWFILE_PROTOCOL_ARTNET)
	builder.AddComment("Art-Net");
	builder.Add(ShowFileParamsConst::ARTNET_DISABLE_UNICAST, static_cast<uint32_t>(m_Params.nDisableUnicast), isMaskSet(showfileparams::Mask::ARTNET_UNICAST_DISABLED));
# endif
#endif

	builder.AddComment("Options");
	builder.Add(ShowFileParamsConst::OPTION_AUTO_PLAY, isMaskSet(showfileparams::Mask::OPTION_AUTO_PLAY), isMaskSet(showfileparams::Mask::OPTION_AUTO_PLAY));
	builder.Add(ShowFileParamsConst::OPTION_LOOP, isMaskSet(showfileparams::Mask::OPTION_LOOP), isMaskSet(showfileparams::Mask::OPTION_LOOP));
#if !defined (CONFIG_SHOWFILE_PROTOCOL_INTERNAL)
	builder.Add(ShowFileParamsConst::OPTION_DISABLE_SYNC, isMaskSet(showfileparams::Mask::OPTION_DISABLE_SYNC), isMaskSet(showfileparams::Mask::OPTION_DISABLE_SYNC));
#endif

#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
	builder.AddComment("OSC Server");
	builder.Add(OscParamsConst::INCOMING_PORT, static_cast<uint32_t>(m_Params.nOscPortIncoming), isMaskSet(showfileparams::Mask::OSC_PORT_INCOMING));
	builder.Add(OscParamsConst::OUTGOING_PORT, static_cast<uint32_t>(m_Params.nOscPortOutgoing), isMaskSet(showfileparams::Mask::OSC_PORT_OUTGOING));
#endif

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void ShowFileParams::Set() {
	DEBUG_ENTRY

	if (isMaskSet(showfileparams::Mask::SHOW)) {
		ShowFile::Get()->SetPlayerShowFileCurrent(m_Params.nShow);
	}

#if defined (CONFIG_SHOWFILE_ENABLE_MASTER)
	if (isMaskSet(showfileparams::Mask::DMX_MASTER)) {
		ShowFile::Get()->SetMaster(m_Params.nDmxMaster);
	}
#endif

#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
	if (isMaskSet(showfileparams::Mask::OSC_PORT_INCOMING)) {
		ShowFile::Get()->SetOscPortIncoming(m_Params.nOscPortIncoming);
	}

	if (isMaskSet(showfileparams::Mask::OSC_PORT_OUTGOING)) {
		ShowFile::Get()->SetOscPortOutgoing(m_Params.nOscPortOutgoing);
	}
#endif

#if !defined (CONFIG_SHOWFILE_PROTOCOL_INTERNAL)
# if defined (CONFIG_SHOWFILE_PROTOCOL_E131)
	if (isMaskSet(showfileparams::Mask::SACN_UNIVERSE)) {
		if (E131Controller::Get() != nullptr) {
			E131Controller::Get()->SetSynchronizationAddress(m_Params.nUniverse);
		}
	}
# endif
# if defined (CONFIG_SHOWFILE_PROTOCOL_ARTNET)
	if (isMaskSet(showfileparams::Mask::ARTNET_UNICAST_DISABLED)) {
		if (ArtNetController::Get() != nullptr) {
			ArtNetController::Get()->SetUnicast(false);
		}
	}
# endif
#endif

	// Options

	if (isMaskSet(showfileparams::Mask::OPTION_AUTO_PLAY)) {
		ShowFile::Get()->SetAutoStart(true);
	}

	if (isMaskSet(showfileparams::Mask::OPTION_LOOP)) {
		ShowFile::Get()->DoLoop(true);
	}

#if !defined (CONFIG_SHOWFILE_PROTOCOL_INTERNAL)
	if (isMaskSet(showfileparams::Mask::OPTION_DISABLE_SYNC)) {
# if defined (CONFIG_SHOWFILE_PROTOCOL_E131)
		if (E131Controller::Get() != nullptr) {
			E131Controller::Get()->SetSynchronizationAddress(0);
		}
# endif
# if defined (CONFIG_SHOWFILE_PROTOCOL_ARTNET)
		if (ArtNetController::Get() != nullptr) {
			ArtNetController::Get()->SetSynchronization(false);
		}
# endif
	}
#endif

	DEBUG_EXIT
}

void ShowFileParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ShowFileParams *>(p))->callbackFunction(s);
}

void ShowFileParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, ShowFileParamsConst::FILE_NAME);
	printf(" %s=%u\n", ShowFileParamsConst::SHOW, m_Params.nShow);

#if defined (CONFIG_SHOWFILE_ENABLE_MASTER)
	printf(" %s=%u\n", ShowFileParamsConst::DMX_MASTER, m_Params.nDmxMaster);
#endif

#if !defined (CONFIG_SHOWFILE_PROTOCOL_INTERNAL)
# if defined (CONFIG_SHOWFILE_PROTOCOL_E131)
	if (isMaskSet(showfileparams::Mask::SACN_UNIVERSE)) {
		printf(" %s=%u\n", ShowFileParamsConst::SACN_SYNC_UNIVERSE, m_Params.nUniverse);
	}
# endif
# if defined (CONFIG_SHOWFILE_PROTOCOL_ARTNET)
	if (isMaskSet(showfileparams::Mask::ARTNET_UNICAST_DISABLED)) {
		printf(" %s=%u [%s]\n", ShowFileParamsConst::ARTNET_DISABLE_UNICAST, m_Params.nDisableUnicast, m_Params.nDisableUnicast == 0 ? "No" : "Yes");
	}
# endif
#endif

	if (isMaskSet(showfileparams::Mask::OPTION_AUTO_PLAY)) {
		printf("  Auto start is enabled\n");
	}

	if (isMaskSet(showfileparams::Mask::OPTION_LOOP)) {
		printf("  Loop is enabled\n");
	}
#if !defined (CONFIG_SHOWFILE_PROTOCOL_INTERNAL)
	if (isMaskSet(showfileparams::Mask::OPTION_DISABLE_SYNC)) {
		printf("  Synchronization is disabled\n");
	}
#endif
#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
	printf(" %s=%u\n", OscParamsConst::INCOMING_PORT, m_Params.nOscPortIncoming);
	printf(" %s=%u\n", OscParamsConst::OUTGOING_PORT, m_Params.nOscPortOutgoing);
#endif
}
