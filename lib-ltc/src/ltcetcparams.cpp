/**
 * @file ltcetcparams.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "ltcetcparams.h"
#include "ltcetcparamsconst.h"
#include "ltcetc.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

LtcEtcParams::LtcEtcParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct ltcetcparams::Params));

	DEBUG_EXIT
}

void LtcEtcParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(LtcEtcParams::StaticCallbackFunction, this);

	if (configfile.Read(LtcEtcParamsConst::FILE_NAME)) {
		LtcEtcParamsStore::Update(&m_Params);
	} else
#endif
		LtcEtcParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void LtcEtcParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(LtcEtcParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	LtcEtcParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void LtcEtcParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, LtcEtcParamsConst::DESTINATION_IP, nValue32) == Sscan::OK) {
		if ((net::is_private_ip(nValue32)) || net::is_multicast_ip(nValue32)) {
			m_Params.nDestinationIp = nValue32;
			m_Params.nSetList |= ltcetcparams::Mask::DESTINATION_IP;
		} else {
			m_Params.nDestinationIp = 0;
			m_Params.nSetList &= ~ltcetcparams::Mask::DESTINATION_IP;
		}
		return;
	}

	if (Sscan::IpAddress(pLine, LtcEtcParamsConst::SOURCE_MULTICAST_IP, nValue32) == Sscan::OK) {
		if (net::is_multicast_ip(nValue32)) {
			m_Params.nSourceMulticastIp = nValue32;
			m_Params.nSetList |= ltcetcparams::Mask::SOURCE_MULTICAST_IP;
		} else {
			m_Params.nDestinationIp = 0;
			m_Params.nSetList &= ~ltcetcparams::Mask::SOURCE_MULTICAST_IP;
		}
		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, LtcEtcParamsConst::DESTINATION_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nDestinationPort = nValue16;
			m_Params.nSetList |= ltcetcparams::Mask::DESTINATION_PORT;
		} else {
			m_Params.nDestinationPort = 0;
			m_Params.nSetList &= ~ltcetcparams::Mask::DESTINATION_PORT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, LtcEtcParamsConst::SOURCE_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nSourcePort = nValue16;
			m_Params.nSetList |= ltcetcparams::Mask::SOURCE_PORT;
		} else {
			m_Params.nSourcePort = 0;
			m_Params.nSetList &= ~ltcetcparams::Mask::SOURCE_PORT;
		}
		return;
	}

	char aTerminator[6];
	uint32_t nLength = sizeof(aTerminator) - 1;

	if (Sscan::Char(pLine, LtcEtcParamsConst::UDP_TERMINATOR, aTerminator, nLength) == Sscan::OK) {
		aTerminator[nLength] = '\0';
		const auto terminator = ltcetc::get_udp_terminator(aTerminator);

		if ((terminator == ltcetc::UdpTerminator::NONE) || (terminator == ltcetc::UdpTerminator::UNDEFINED)) {
			m_Params.nUdpTerminator = 0;
			m_Params.nSetList &= ~ltcetcparams::Mask::UDP_TERMINATOR;
		} else {
			m_Params.nUdpTerminator = static_cast<uint8_t>(terminator);
			m_Params.nSetList |= ltcetcparams::Mask::UDP_TERMINATOR;
		}
		return;
	}

}

void LtcEtcParams::Builder(const struct ltcetcparams::Params *ptLtcEtcParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != nullptr);

	if (ptLtcEtcParams != nullptr) {
		memcpy(&m_Params, ptLtcEtcParams, sizeof(struct ltcetcparams::Params));
	} else {
		LtcEtcParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(LtcEtcParamsConst::FILE_NAME, pBuffer, nLength);

	builder.AddComment("Out");
	builder.AddIpAddress(LtcEtcParamsConst::DESTINATION_IP, m_Params.nDestinationIp, isMaskSet(ltcetcparams::Mask::DESTINATION_IP));
	builder.Add(LtcEtcParamsConst::DESTINATION_PORT, m_Params.nDestinationPort, isMaskSet(ltcetcparams::Mask::DESTINATION_PORT));

	builder.AddComment("In");
	builder.AddIpAddress(LtcEtcParamsConst::SOURCE_MULTICAST_IP, m_Params.nSourceMulticastIp, isMaskSet(ltcetcparams::Mask::SOURCE_MULTICAST_IP));
	builder.Add(LtcEtcParamsConst::SOURCE_PORT, m_Params.nSourcePort, isMaskSet(ltcetcparams::Mask::SOURCE_PORT));

	builder.AddComment("UDP Terminator: None/CR/LF/CRLF");
	builder.Add(LtcEtcParamsConst::UDP_TERMINATOR, ltcetc::get_udp_terminator(static_cast<ltcetc::UdpTerminator>(m_Params.nUdpTerminator)), isMaskSet(ltcetcparams::Mask::UDP_TERMINATOR));

	nSize = builder.GetSize();
}

void LtcEtcParams::Set() {
	auto *p = LtcEtc::Get();

	if (isMaskSet(ltcetcparams::Mask::DESTINATION_IP)) {
		p->SetDestinationIp(m_Params.nDestinationIp);
	}

	if (isMaskSet(ltcetcparams::Mask::DESTINATION_PORT)) {
		p->SetDestinationPort(m_Params.nDestinationPort);
	}

	if (isMaskSet(ltcetcparams::Mask::SOURCE_MULTICAST_IP)) {
		p->SetSourceMulticastIp(m_Params.nSourceMulticastIp);
	}

	if (isMaskSet(ltcetcparams::Mask::SOURCE_PORT)) {
		p->SetSourcePort(m_Params.nSourcePort);
	}

	if (isMaskSet(ltcetcparams::Mask::UDP_TERMINATOR)) {
		p->SetUdpTerminator(static_cast<ltcetc::UdpTerminator>(m_Params.nUdpTerminator));
	}
}

void LtcEtcParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<LtcEtcParams *>(p))->callbackFunction(s);
}

void LtcEtcParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcEtcParamsConst::FILE_NAME);

	if (isMaskSet(ltcetcparams::Mask::DESTINATION_IP)) {
		printf(" %s=" IPSTR "\n", LtcEtcParamsConst::DESTINATION_IP, IP2STR(m_Params.nDestinationIp));
	}

	if (isMaskSet(ltcetcparams::Mask::DESTINATION_PORT)) {
		printf(" %s=%u\n", LtcEtcParamsConst::DESTINATION_PORT, m_Params.nDestinationPort);
	}

	if (isMaskSet(ltcetcparams::Mask::SOURCE_MULTICAST_IP)) {
		printf(" %s=" IPSTR "\n", LtcEtcParamsConst::SOURCE_MULTICAST_IP, IP2STR(m_Params.nSourceMulticastIp));
	}

	if (isMaskSet(ltcetcparams::Mask::SOURCE_PORT)) {
		printf(" %s=%u\n", LtcEtcParamsConst::SOURCE_PORT, m_Params.nSourcePort);
	}

	if (isMaskSet(ltcetcparams::Mask::UDP_TERMINATOR)) {
		printf(" %s=%s [%u]\n", LtcEtcParamsConst::UDP_TERMINATOR, ltcetc::get_udp_terminator(static_cast<ltcetc::UdpTerminator>(m_Params.nUdpTerminator)) , m_Params.nUdpTerminator);
	}
}
