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

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "showfileparams.h"
#include "showfileparamsconst.h"
#include "showfile.h"
#include "showfileconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "showfileosc.h"
#include "oscconst.h"

// Protocols
#include "e131controller.h"
#include "artnetcontroller.h"

#include "debug.h"

#define PROTOCOL2STRING(p)		(p == SHOWFILE_PROTOCOL_SACN) ? "sACN" : "Art-Net"

ShowFileParams::ShowFileParams(ShowFileParamsStore *pShowFileParamsStore): m_pShowFileParamsStore(pShowFileParamsStore) {
	DEBUG_ENTRY

	m_tShowFileParams.nSetList = 0;
	m_tShowFileParams.nFormat = SHOWFILE_FORMAT_OLA;
	m_tShowFileParams.nShow = 0;
	m_tShowFileParams.nOptions = 0;
	m_tShowFileParams.nOscPortIncoming = OSCSERVER_PORT_DEFAULT_INCOMING;
	m_tShowFileParams.nOscPortOutgoing = OSCSERVER_PORT_DEFAULT_OUTGOING;
	m_tShowFileParams.nProtocol = SHOWFILE_PROTOCOL_SACN;
	m_tShowFileParams.nUniverse = DEFAULT_SYNCHRONIZATION_ADDRESS;
	m_tShowFileParams.nDisableUnicast = 0;
	m_tShowFileParams.nDmxMaster = DMX_MAX_VALUE;

	DEBUG_EXIT
}

ShowFileParams::~ShowFileParams(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

bool ShowFileParams::Load(void) {
	m_tShowFileParams.nSetList = 0;

	ReadConfigFile configfile(ShowFileParams::staticCallbackFunction, this);

	if (configfile.Read(ShowFileParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pShowFileParamsStore != 0) {
			m_pShowFileParamsStore->Update(&m_tShowFileParams);
		}
	} else if (m_pShowFileParamsStore != 0) {
		m_pShowFileParamsStore->Copy(&m_tShowFileParams);
	} else {
		return false;
	}

	return true;
}

void ShowFileParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);

	assert(m_pShowFileParamsStore != 0);

	if (m_pShowFileParamsStore == 0) {
		return;
	}

	m_tShowFileParams.nSetList = 0;

	ReadConfigFile config(ShowFileParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pShowFileParamsStore->Update(&m_tShowFileParams);
}

void ShowFileParams::HandleOptions(const char *pLine, const char *pKeyword, TShowFileOptions tShowFileOptions) {
	uint8_t value8;

	if (Sscan::Uint8(pLine, pKeyword, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tShowFileParams.nOptions |= (uint8_t) tShowFileOptions;
			m_tShowFileParams.nSetList |= SHOWFILE_PARAMS_MASK_OPTIONS;
		} else {
			m_tShowFileParams.nOptions &= ~((uint8_t) tShowFileOptions);
		}

		if (m_tShowFileParams.nOptions == 0) {
			m_tShowFileParams.nSetList &= ~SHOWFILE_PARAMS_MASK_OPTIONS;
		}
	}
}

void ShowFileParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	char value[16];
	uint8_t nLength;
	uint8_t value8;
	uint16_t value16;

	nLength = SHOWFILECONST_FORMAT_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ShowFileParamsConst::FORMAT, value, &nLength) == SSCAN_OK) {
		value[nLength] = '\0';

		TShowFileFormats tFormat = ShowFile::GetFormat(value);

		if (tFormat != SHOWFILE_FORMAT_UNDEFINED) {
			m_tShowFileParams.nFormat = tFormat;
			m_tShowFileParams.nSetList |= SHOWFILE_PARAMS_MASK_FORMAT;
		} else {
			m_tShowFileParams.nFormat = SHOWFILE_FORMAT_OLA;
			m_tShowFileParams.nSetList &= ~SHOWFILE_PARAMS_MASK_FORMAT;
		}

		return;
	}

	if (Sscan::Uint16(pLine, OscConst::PARAMS_INCOMING_PORT, &value16) == SSCAN_OK) {
		if (value16 > 1023) {
			m_tShowFileParams.nOscPortIncoming = value16;
			m_tShowFileParams.nSetList |= SHOWFILE_PARAMS_MASK_OSC_PORT_INCOMING;
		} else {
			m_tShowFileParams.nOscPortIncoming = OSCSERVER_PORT_DEFAULT_INCOMING;
			m_tShowFileParams.nSetList &= ~SHOWFILE_PARAMS_MASK_OSC_PORT_INCOMING;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscConst::PARAMS_OUTGOING_PORT, &value16) == SSCAN_OK) {
		if (value16 > 1023) {
			m_tShowFileParams.nOscPortOutgoing = value16;
			m_tShowFileParams.nSetList |= SHOWFILE_PARAMS_MASK_OSC_PORT_OUTGOING;
		} else {
			m_tShowFileParams.nOscPortOutgoing = OSCSERVER_PORT_DEFAULT_OUTGOING;
			m_tShowFileParams.nSetList &= ~SHOWFILE_PARAMS_MASK_OSC_PORT_OUTGOING;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ShowFileParamsConst::SHOW, &value8) == SSCAN_OK) {
		if (value8 < SHOWFILE_FILE_MAX_NUMBER) {
			m_tShowFileParams.nShow = value8;
			m_tShowFileParams.nSetList |= SHOWFILE_PARAMS_MASK_SHOW;
		} else {
			m_tShowFileParams.nShow = 0;
			m_tShowFileParams.nSetList &= ~SHOWFILE_PARAMS_MASK_SHOW;
		}
		return;
	}

	nLength = 6;
	if (Sscan::Char(pLine, ShowFileParamsConst::PROTOCOL, value, &nLength) == SSCAN_OK) {
		value[nLength] = '\0';

		if(strcasecmp(value, "artnet") == 0) {
			m_tShowFileParams.nProtocol = SHOWFILE_PROTOCOL_ARTNET;
			m_tShowFileParams.nSetList |= SHOWFILE_PARAMS_MASK_PROTOCOL;
		} else {
			m_tShowFileParams.nProtocol = SHOWFILE_PROTOCOL_SACN;
			m_tShowFileParams.nSetList &= SHOWFILE_PARAMS_MASK_PROTOCOL;
		}
		return;
	}

	if (Sscan::Uint16(pLine, ShowFileParamsConst::SACN_SYNC_UNIVERSE, &value16) == SSCAN_OK) {
		if (value16 > E131_UNIVERSE_MAX) {
			m_tShowFileParams.nUniverse = DEFAULT_SYNCHRONIZATION_ADDRESS;
			m_tShowFileParams.nSetList &= ~SHOWFILE_PARAMS_MASK_SACN_UNIVERSE;
		} else {
			m_tShowFileParams.nUniverse = value16;
			m_tShowFileParams.nSetList |= SHOWFILE_PARAMS_MASK_SACN_UNIVERSE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ShowFileParamsConst::ARTNET_DISABLE_UNICAST, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tShowFileParams.nDisableUnicast = 1;
			m_tShowFileParams.nSetList |= SHOWFILE_PARAMS_MASK_ARTNET_UNICAST;
		} else {
			m_tShowFileParams.nDisableUnicast = 0;
			m_tShowFileParams.nSetList &= ~SHOWFILE_PARAMS_MASK_ARTNET_UNICAST;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ShowFileParamsConst::DMX_MASTER, &value8) == SSCAN_OK) {
		if (value8 < DMX_MAX_VALUE) {
			m_tShowFileParams.nDmxMaster = value8;
			m_tShowFileParams.nSetList |= SHOWFILE_PARAMS_MASK_DMX_MASTER;
		} else {
			m_tShowFileParams.nDmxMaster = DMX_MAX_VALUE;
			m_tShowFileParams.nSetList &= ~SHOWFILE_PARAMS_MASK_DMX_MASTER;
		}
		return;
	}

	HandleOptions(pLine, ShowFileParamsConst::OPTION_AUTO_START, SHOWFILE_OPTION_AUTO_START);
	HandleOptions(pLine, ShowFileParamsConst::OPTION_LOOP, SHOWFILE_OPTION_LOOP);
}

void ShowFileParams::Builder(const struct TShowFileParams *ptShowFileParamss, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (ptShowFileParamss != 0) {
		memcpy(&m_tShowFileParams, ptShowFileParamss, sizeof(struct TShowFileParams));
	} else {
		m_pShowFileParamsStore->Copy(&m_tShowFileParams);
	}

	PropertiesBuilder builder(ShowFileParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(ShowFileParamsConst::SHOW, (uint32_t) m_tShowFileParams.nShow, isMaskSet(SHOWFILE_PARAMS_MASK_SHOW));

	builder.Add(ShowFileParamsConst::FORMAT, ShowFile::GetFormat((TShowFileFormats) m_tShowFileParams.nFormat), isMaskSet(SHOWFILE_PARAMS_MASK_FORMAT));
	builder.Add(ShowFileParamsConst::PROTOCOL, (m_tShowFileParams.nProtocol == SHOWFILE_PROTOCOL_SACN) ? "sacn" : "artnet", isMaskSet(SHOWFILE_PARAMS_MASK_PROTOCOL));

	builder.AddComment("DMX");
	builder.Add(ShowFileParamsConst::DMX_MASTER, (uint32_t) m_tShowFileParams.nDmxMaster, isMaskSet(SHOWFILE_PARAMS_MASK_DMX_MASTER));

	builder.AddComment("sACN");
	builder.Add(ShowFileParamsConst::SACN_SYNC_UNIVERSE, (uint32_t) m_tShowFileParams.nUniverse, isMaskSet(SHOWFILE_PARAMS_MASK_SACN_UNIVERSE));

	builder.AddComment("Options");
	builder.Add(ShowFileParamsConst::OPTION_AUTO_START, isOptionSet(SHOWFILE_OPTION_AUTO_START), isOptionSet(SHOWFILE_OPTION_AUTO_START));
	builder.Add(ShowFileParamsConst::OPTION_LOOP, isOptionSet(SHOWFILE_OPTION_LOOP), isOptionSet(SHOWFILE_OPTION_LOOP));
	builder.Add(ShowFileParamsConst::OPTION_DISABLE_SYNC, isOptionSet(SHOWFILE_OPTION_DISABLE_SYNC), isOptionSet(SHOWFILE_OPTION_DISABLE_SYNC));

	builder.AddComment("OSC Server");
	builder.Add(OscConst::PARAMS_INCOMING_PORT, (uint32_t) m_tShowFileParams.nOscPortIncoming, isMaskSet(SHOWFILE_PARAMS_MASK_OSC_PORT_INCOMING));
	builder.Add(OscConst::PARAMS_OUTGOING_PORT, (uint32_t) m_tShowFileParams.nOscPortOutgoing, isMaskSet(SHOWFILE_PARAMS_MASK_OSC_PORT_OUTGOING));

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void ShowFileParams::Save(uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pShowFileParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(0, pBuffer, nLength, nSize);

	DEBUG_EXIT
	return;
}

void ShowFileParams::Set(void) {

	if (isMaskSet(SHOWFILE_PARAMS_MASK_OSC_PORT_INCOMING)) {
		ShowFileOSC::Get()->SetPortIncoming(m_tShowFileParams.nOscPortIncoming);
	}

	if (isMaskSet(SHOWFILE_PARAMS_MASK_OSC_PORT_OUTGOING)) {
		ShowFileOSC::Get()->SetPortOutgoing(m_tShowFileParams.nOscPortOutgoing);
	}

	if (isMaskSet(SHOWFILE_PARAMS_MASK_SACN_UNIVERSE)) {
		if (E131Controller::Get() != 0) {
			E131Controller::Get()->SetSynchronizationAddress(m_tShowFileParams.nUniverse);
		}
	}

	if (isOptionSet(SHOWFILE_OPTION_LOOP)) {
		ShowFile::Get()->DoLoop(true);
	}

	if (isOptionSet(SHOWFILE_OPTION_DISABLE_SYNC)) {
		if (E131Controller::Get() != 0) {
			E131Controller::Get()->SetSynchronizationAddress(0);
		}
		if (ArtNetController::Get() != 0) {
			ArtNetController::Get()->SetSynchronization(false);
		}
	}

	if (isOptionSet(SHOWFILE_OPTION_DISABLE_UNICAST)) {
		if (ArtNetController::Get() != 0) {
			ArtNetController::Get()->SetUnicast(false);
		}
	}
}

void ShowFileParams::Dump(void) {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, ShowFileParamsConst::FILE_NAME);

	if(isMaskSet(SHOWFILE_PARAMS_MASK_FORMAT)) {
		printf(" %s=%d [%s]\n", ShowFileParamsConst::FORMAT, m_tShowFileParams.nFormat, ShowFile::GetFormat((TShowFileFormats) m_tShowFileParams.nFormat));
	}

	if (isMaskSet(SHOWFILE_PARAMS_MASK_SHOW)) {
		printf(" %s=%u\n", ShowFileParamsConst::SHOW, (unsigned) m_tShowFileParams.nShow);
	}

	if(isMaskSet(SHOWFILE_PARAMS_MASK_PROTOCOL)) {
		printf(" %s=%d [%s]\n", ShowFileParamsConst::PROTOCOL, (unsigned) m_tShowFileParams.nProtocol, PROTOCOL2STRING(m_tShowFileParams.nProtocol));
	}

	if (isMaskSet(SHOWFILE_PARAMS_MASK_DMX_MASTER)) {
		printf(" %s=%u\n", ShowFileParamsConst::DMX_MASTER, (unsigned) m_tShowFileParams.nDmxMaster);
	}

	if (isMaskSet(SHOWFILE_PARAMS_MASK_SACN_UNIVERSE)) {
		printf(" %s=%u\n", ShowFileParamsConst::SACN_SYNC_UNIVERSE, (unsigned) m_tShowFileParams.nUniverse);
	}

	if (isMaskSet(SHOWFILE_PARAMS_MASK_ARTNET_UNICAST)) {
		printf(" %s=%u [%s]\n", ShowFileParamsConst::ARTNET_DISABLE_UNICAST, (unsigned) m_tShowFileParams.nDisableUnicast, m_tShowFileParams.nDisableUnicast == 0 ? "No" : "Yes");
	}

	// Options
	if (isMaskSet(SHOWFILE_PARAMS_MASK_OPTIONS)) {
		printf(" Options 0x%.2x\n", m_tShowFileParams.nOptions);

		if (isOptionSet(SHOWFILE_OPTION_AUTO_START)) {
			printf("  Auto start is enabled\n");
		}
		if (isOptionSet(SHOWFILE_OPTION_LOOP)) {
			printf("  Loop is enabled\n");
		}
		if (isOptionSet(SHOWFILE_OPTION_DISABLE_SYNC)) {
			printf("  Synchronization is disabled\n");
		}
		if (isOptionSet(SHOWFILE_OPTION_DISABLE_UNICAST)) {
			printf("  Unicast is disabled [Art-Net DMX broadcast]\n");
		}
	}

	if (isMaskSet(SHOWFILE_PARAMS_MASK_OSC_PORT_INCOMING)) {
		printf(" %s=%u\n", OscConst::PARAMS_INCOMING_PORT, (unsigned) m_tShowFileParams.nOscPortIncoming);
	}

	if (isMaskSet(SHOWFILE_PARAMS_MASK_OSC_PORT_OUTGOING)) {
		printf(" %s=%u\n", OscConst::PARAMS_OUTGOING_PORT, (unsigned) m_tShowFileParams.nOscPortOutgoing);
	}
#endif
}

void ShowFileParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((ShowFileParams *) p)->callbackFunction(s);
}

