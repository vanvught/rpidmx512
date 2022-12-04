/**
 * @file ltcetcparams.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "ltcetcparams.h"
#include "ltcetcparamsconst.h"
#include "ltcetc.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

LtcEtcParams::LtcEtcParams(LtcEtcParamsStore *pLtcEtcParamsStore): m_pLtcEtcParamsStore(pLtcEtcParamsStore) {
	memset(&m_pLtcEtcParams, 0, sizeof(struct ltcetcparams::Params));
}

bool LtcEtcParams::Load() {
	m_pLtcEtcParams.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(LtcEtcParams::staticCallbackFunction, this);

	if (configfile.Read(LtcEtcParamsConst::FILE_NAME)) {
		if (m_pLtcEtcParamsStore != nullptr) {
			m_pLtcEtcParamsStore->Update(&m_pLtcEtcParams);
		}
	} else
#endif
	if (m_pLtcEtcParamsStore != nullptr) {
		m_pLtcEtcParamsStore->Copy(&m_pLtcEtcParams);
	} else {
		return false;
	}

	return true;
}

void LtcEtcParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);

	assert(m_pLtcEtcParamsStore != nullptr);

	if (m_pLtcEtcParamsStore == nullptr) {
		return;
	}

	m_pLtcEtcParams.nSetList = 0;

	ReadConfigFile config(LtcEtcParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pLtcEtcParamsStore->Update(&m_pLtcEtcParams);
}

void LtcEtcParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, LtcEtcParamsConst::DESTINATION_IP, nValue32) == Sscan::OK) {
		if ((network::is_private_ip(nValue32)) || network::is_multicast_ip(nValue32)) {
			m_pLtcEtcParams.nDestinationIp = nValue32;
			m_pLtcEtcParams.nSetList |= ltcetcparams::Mask::DESTINATION_IP;
		} else {
			m_pLtcEtcParams.nDestinationIp = 0;
			m_pLtcEtcParams.nSetList &= ~ltcetcparams::Mask::DESTINATION_IP;
		}
		return;
	}

	if (Sscan::IpAddress(pLine, LtcEtcParamsConst::SOURCE_MULTICAST_IP, nValue32) == Sscan::OK) {
		if (network::is_multicast_ip(nValue32)) {
			m_pLtcEtcParams.nSourceMulticastIp = nValue32;
			m_pLtcEtcParams.nSetList |= ltcetcparams::Mask::SOURCE_MULTICAST_IP;
		} else {
			m_pLtcEtcParams.nDestinationIp = 0;
			m_pLtcEtcParams.nSetList &= ~ltcetcparams::Mask::SOURCE_MULTICAST_IP;
		}
		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, LtcEtcParamsConst::DESTINATION_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_pLtcEtcParams.nDestinationPort = nValue16;
			m_pLtcEtcParams.nSetList |= ltcetcparams::Mask::DESTINATION_PORT;
		} else {
			m_pLtcEtcParams.nDestinationPort = 0;
			m_pLtcEtcParams.nSetList &= ~ltcetcparams::Mask::DESTINATION_PORT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, LtcEtcParamsConst::SOURCE_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_pLtcEtcParams.nSourcePort = nValue16;
			m_pLtcEtcParams.nSetList |= ltcetcparams::Mask::SOURCE_PORT;
		} else {
			m_pLtcEtcParams.nSourcePort = 0;
			m_pLtcEtcParams.nSetList &= ~ltcetcparams::Mask::SOURCE_PORT;
		}
		return;
	}

	char aTerminator[6];
	uint32_t nLength = sizeof(aTerminator) - 1;

	if (Sscan::Char(pLine, LtcEtcParamsConst::UDP_TERMINATOR, aTerminator, nLength) == Sscan::OK) {
		aTerminator[nLength] = '\0';
		const auto terminator = ltc::etc::get_udp_terminator(aTerminator);

		if ((terminator == ltc::etc::UdpTerminator::NONE) || (terminator == ltc::etc::UdpTerminator::UNDEFINED)) {
			m_pLtcEtcParams.nUdpTerminator = 0;
			m_pLtcEtcParams.nSetList &= ~ltcetcparams::Mask::UDP_TERMINATOR;
		} else {
			m_pLtcEtcParams.nUdpTerminator = static_cast<uint8_t>(terminator);
			m_pLtcEtcParams.nSetList |= ltcetcparams::Mask::UDP_TERMINATOR;
		}
		return;
	}

}

void LtcEtcParams::Builder(const struct ltcetcparams::Params *ptLtcEtcParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != nullptr);

	if (ptLtcEtcParams != nullptr) {
		memcpy(&m_pLtcEtcParams, ptLtcEtcParams, sizeof(struct ltcetcparams::Params));
	} else {
		m_pLtcEtcParamsStore->Copy(&m_pLtcEtcParams);
	}

	PropertiesBuilder builder(LtcEtcParamsConst::FILE_NAME, pBuffer, nLength);

	builder.AddComment("Out");
	builder.AddIpAddress(LtcEtcParamsConst::DESTINATION_IP, m_pLtcEtcParams.nDestinationIp, isMaskSet(ltcetcparams::Mask::DESTINATION_IP));
	builder.Add(LtcEtcParamsConst::DESTINATION_PORT, m_pLtcEtcParams.nDestinationPort, isMaskSet(ltcetcparams::Mask::DESTINATION_PORT));

	builder.AddComment("In");
	builder.AddIpAddress(LtcEtcParamsConst::SOURCE_MULTICAST_IP, m_pLtcEtcParams.nSourceMulticastIp, isMaskSet(ltcetcparams::Mask::SOURCE_MULTICAST_IP));
	builder.Add(LtcEtcParamsConst::SOURCE_PORT, m_pLtcEtcParams.nSourcePort, isMaskSet(ltcetcparams::Mask::SOURCE_PORT));

	builder.AddComment("UDP Terminator: None/CR/LF/CRLF");
	builder.Add(LtcEtcParamsConst::UDP_TERMINATOR, ltc::etc::get_udp_terminator(static_cast<ltc::etc::UdpTerminator>(m_pLtcEtcParams.nUdpTerminator)), isMaskSet(ltcetcparams::Mask::UDP_TERMINATOR));

	nSize = builder.GetSize();
}

void LtcEtcParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	if (m_pLtcEtcParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void LtcEtcParams::Set() {
	auto *p = LtcEtc::Get();

	if (isMaskSet(ltcetcparams::Mask::DESTINATION_IP)) {
		p->SetDestinationIp(m_pLtcEtcParams.nDestinationIp);
	}

	if (isMaskSet(ltcetcparams::Mask::DESTINATION_PORT)) {
		p->SetDestinationPort(m_pLtcEtcParams.nDestinationPort);
	}

	if (isMaskSet(ltcetcparams::Mask::SOURCE_MULTICAST_IP)) {
		p->SetSourceMulticastIp(m_pLtcEtcParams.nSourceMulticastIp);
	}

	if (isMaskSet(ltcetcparams::Mask::SOURCE_PORT)) {
		p->SetSourcePort(m_pLtcEtcParams.nSourcePort);
	}

	if (isMaskSet(ltcetcparams::Mask::UDP_TERMINATOR)) {
		p->SetUdpTerminator(static_cast<ltc::etc::UdpTerminator>(m_pLtcEtcParams.nUdpTerminator));
	}
}

void LtcEtcParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<LtcEtcParams *>(p))->callbackFunction(s);
}

#include <cstdio>

#include "network.h"

void LtcEtcParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcEtcParamsConst::FILE_NAME);

	if (isMaskSet(ltcetcparams::Mask::DESTINATION_IP)) {
		printf(" %s=" IPSTR "\n", LtcEtcParamsConst::DESTINATION_IP, IP2STR(m_pLtcEtcParams.nDestinationIp));
	}

	if (isMaskSet(ltcetcparams::Mask::DESTINATION_PORT)) {
		printf(" %s=%u\n", LtcEtcParamsConst::DESTINATION_PORT, m_pLtcEtcParams.nDestinationPort);
	}

	if (isMaskSet(ltcetcparams::Mask::SOURCE_MULTICAST_IP)) {
		printf(" %s=" IPSTR "\n", LtcEtcParamsConst::SOURCE_MULTICAST_IP, IP2STR(m_pLtcEtcParams.nSourceMulticastIp));
	}

	if (isMaskSet(ltcetcparams::Mask::SOURCE_PORT)) {
		printf(" %s=%u\n", LtcEtcParamsConst::SOURCE_PORT, m_pLtcEtcParams.nSourcePort);
	}

	if (isMaskSet(ltcetcparams::Mask::UDP_TERMINATOR)) {
		printf(" %s=%s [%u]\n", LtcEtcParamsConst::UDP_TERMINATOR, ltc::etc::get_udp_terminator(static_cast<ltc::etc::UdpTerminator>(m_pLtcEtcParams.nUdpTerminator)) , m_pLtcEtcParams.nUdpTerminator);
	}
#endif
}
