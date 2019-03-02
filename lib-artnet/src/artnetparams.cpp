/**
 * @file artnetparams.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "artnetparams.h"
#include "artnetnode.h"

#include "readconfigfile.h"
#include "sscan.h"

#define BOOL2STRING(b)			(b) ? "Yes" : "No"
#define MERGEMODE2STRING(m)		(m == ARTNET_MERGE_HTP) ? "HTP" : "LTP"
#define PROTOCOL2STRING(p)		(p == PORT_ARTNET_ARTNET) ? "Art-Net" : "sACN"

static const char PARAMS_FILE_NAME[] ALIGNED = "artnet.txt";
static const char PARAMS_NET[] ALIGNED = "net";												///< 0 {default}
static const char PARAMS_SUBNET[] ALIGNED = "subnet";										///< 0 {default}
static const char PARAMS_UNIVERSE[] ALIGNED = "universe";									///< 0 {default}
static const char PARAMS_OUTPUT[] ALIGNED = "output";										///< dmx {default}, spi, mon
static const char PARAMS_TIMECODE[] ALIGNED = "use_timecode";
static const char PARAMS_TIMESYNC[] ALIGNED = "use_timesync";
static const char PARAMS_RDM[] ALIGNED = "enable_rdm";										///< Enable RDM, 0 {default}
static const char PARAMS_RDM_DISCOVERY[] ALIGNED = "rdm_discovery_at_startup";				///< 0 {default}
static const char PARAMS_NODE_SHORT_NAME[] ALIGNED = "short_name";
static const char PARAMS_NODE_LONG_NAME[] ALIGNED = "long_name";
static const char PARAMS_NODE_MANUFACTURER_ID[] ALIGNED = "manufacturer_id";
static const char PARAMS_NODE_OEM_VALUE[] ALIGNED = "oem_value";
static const char PARAMS_NODE_NETWORK_DATA_LOSS_TIMEOUT[] = "network_data_loss_timeout";	///< 10 {default}
static const char PARAMS_NODE_DISABLE_MERGE_TIMEOUT[] = "disable_merge_timeout";			///< 0 {default}
static const char PARAMS_UNIVERSE_PORT[4][16] ALIGNED = { "universe_port_a",
		"universe_port_b", "universe_port_c", "universe_port_d" };
static const char PARAMS_MERGE_MODE[] ALIGNED = "merge_mode";
static const char PARAMS_MERGE_MODE_PORT[4][18] ALIGNED = { "merge_mode_port_a",
		"merge_mode_port_b", "merge_mode_port_c", "merge_mode_port_d" };
static const char PARAMS_PROTOCOL[] ALIGNED = "protocol";
static const char PARAMS_PROTOCOL_PORT[4][16] ALIGNED = { "protocol_port_a",
		"protocol_port_b", "protocol_port_c", "protocol_port_d" };

ArtNetParams::ArtNetParams(ArtNetParamsStore *pArtNetParamsStore): m_pArtNetParamsStore(pArtNetParamsStore) {
	uint8_t *p = (uint8_t *) &m_tArtNetParams;

	for (uint32_t i = 0; i < sizeof(struct TArtNetParams); i++) {
		*p++ = 0;
	}

	for (uint32_t i = 0; i < ARTNET_MAX_PORTS; i++) {
		m_tArtNetParams.nUniversePort[i] = i;
	}
}

ArtNetParams::~ArtNetParams(void) {
}

bool ArtNetParams::Load(void) {
	m_tArtNetParams.nSetList = 0;

	ReadConfigFile configfile(ArtNetParams::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pArtNetParamsStore != 0) {
			m_pArtNetParamsStore->Update(&m_tArtNetParams);
		}
	} else if (m_pArtNetParamsStore != 0) {
		m_pArtNetParamsStore->Copy(&m_tArtNetParams);
	} else {
		return false;
	}

	return true;
}

void ArtNetParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	char value[128];
	uint8_t len;
	uint8_t value8;

	if (Sscan::Uint8(pLine, PARAMS_TIMECODE, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tArtNetParams.bUseTimeCode = true;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_TIMECODE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_TIMESYNC, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tArtNetParams.bUseTimeSync = true;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_TIMESYNC;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_RDM, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tArtNetParams.bEnableRdm = true;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_RDM;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_RDM_DISCOVERY, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tArtNetParams.bRdmDiscovery = true;
		}
		return;
	}

	len = ARTNET_SHORT_NAME_LENGTH;
	if (Sscan::Char(pLine, PARAMS_NODE_SHORT_NAME, value, &len) == SSCAN_OK) {
		strncpy((char *)m_tArtNetParams.aShortName, value, len);
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_SHORT_NAME;
		return;
	}

	len = ARTNET_LONG_NAME_LENGTH;
	if (Sscan::Char(pLine, PARAMS_NODE_LONG_NAME, value, &len) == SSCAN_OK) {
		strncpy((char *)m_tArtNetParams.aLongName, value, len);
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_LONG_NAME;
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_OUTPUT, value, &len) == SSCAN_OK) {
		if (memcmp(value, "spi", 3) == 0) {
			m_tArtNetParams.tOutputType = OUTPUT_TYPE_SPI;
		} else if (memcmp(value, "mon", 3) == 0) {
			m_tArtNetParams.tOutputType = OUTPUT_TYPE_MONITOR;
		} else {
			m_tArtNetParams.tOutputType = OUTPUT_TYPE_DMX;
		}
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_OUTPUT;
		return;
	}

	len = 4;
	if (Sscan::Char(pLine, PARAMS_NODE_MANUFACTURER_ID, value, &len) == SSCAN_OK) {
		if (len == 4) {
			const uint16_t v = HexUint16(value);
			m_tArtNetParams.aManufacturerId[0] = (uint8_t) (v >> 8);
			m_tArtNetParams.aManufacturerId[1] = (uint8_t) (v & 0xFF);
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_ID;
		}
		return;
	}

	len = 4;
	if (Sscan::Char(pLine, PARAMS_NODE_OEM_VALUE, value, &len) == SSCAN_OK) {
		if (len == 4) {
			const uint16_t v = HexUint16(value);
			m_tArtNetParams.aOemValue[0] = (uint8_t) (v >> 8);
			m_tArtNetParams.aOemValue[1] = (uint8_t) (v & 0xFF);
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_OEM_VALUE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_NODE_NETWORK_DATA_LOSS_TIMEOUT, &value8) == SSCAN_OK) {
		m_tArtNetParams.nNetworkTimeout = (time_t) value8;
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_NETWORK_TIMEOUT;
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_NODE_DISABLE_MERGE_TIMEOUT, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tArtNetParams.bDisableMergeTimeout = true;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_MERGE_TIMEOUT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_NET, &value8) == SSCAN_OK) {
		m_tArtNetParams.nNet = value8;
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_NET;
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_SUBNET, &value8) == SSCAN_OK) {
		m_tArtNetParams.nSubnet = value8;
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_SUBNET;
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_UNIVERSE, &value8) == SSCAN_OK) {
		if (value8 <= 0xF) {
			m_tArtNetParams.nUniverse = value8;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_UNIVERSE;
		}
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_MERGE_MODE, value, &len) == SSCAN_OK) {
		if (memcmp(value, "ltp", 3) == 0) {
			m_tArtNetParams.nMergeMode = ARTNET_MERGE_LTP;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_MERGE_MODE;
		} else if (memcmp(value, "htp", 3) == 0) {
			m_tArtNetParams.nMergeMode = ARTNET_MERGE_HTP;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_MERGE_MODE;
		}
		return;
	}

	len = 4;
	if (Sscan::Char(pLine, PARAMS_PROTOCOL, value, &len) == SSCAN_OK) {
		if(memcmp(value, "sacn", 4) == 0) {
			m_tArtNetParams.nProtocol = PORT_ARTNET_SACN;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_PROTOCOL;
		}
		return;
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (Sscan::Uint8(pLine, PARAMS_UNIVERSE_PORT[i], &value8) == SSCAN_OK) {
			m_tArtNetParams.nUniversePort[i] = value8;
			m_tArtNetParams.nSetList |= (ARTNET_PARAMS_MASK_UNIVERSE_A << i);
			return;
		}

		len = 3;
		if (Sscan::Char(pLine, PARAMS_MERGE_MODE_PORT[i], value, &len) == SSCAN_OK) {
			if (memcmp(value, "ltp", 3) == 0) {
				m_tArtNetParams.nMergeModePort[i] = ARTNET_MERGE_LTP;
				m_tArtNetParams.nSetList |= (ARTNET_PARAMS_MASK_MERGE_MODE_A << i);
			} else if (memcmp(value, "htp", 3) == 0) {
				m_tArtNetParams.nMergeModePort[i] = ARTNET_MERGE_HTP;
				m_tArtNetParams.nSetList |= (ARTNET_PARAMS_MASK_MERGE_MODE_A << i);
			}
			return;
		}

		len = 4;
		if (Sscan::Char(pLine, PARAMS_PROTOCOL_PORT[i], value, &len) == SSCAN_OK) {
			if (memcmp(value, "sacn", 4) == 0) {
				m_tArtNetParams.nProtocolPort[i] = PORT_ARTNET_SACN;
				m_tArtNetParams.nSetList |= (ARTNET_PARAMS_MASK_PROTOCOL_A << i);
			}
			return;
		}
	}
}

uint8_t ArtNetParams::GetUniverse(uint8_t nPort, bool& IsSet) const {
	assert(nPort < ARTNET_MAX_PORTS);

	IsSet = isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE_A << nPort);

	return m_tArtNetParams.nUniversePort[nPort];
}

void ArtNetParams::Set(ArtNetNode *pArtNetNode) {
	assert(pArtNetNode != 0);

	if (m_tArtNetParams.nSetList == 0) {
		return;
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_SHORT_NAME)) {
		pArtNetNode->SetShortName((const char *)m_tArtNetParams.aShortName);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_LONG_NAME)) {
		pArtNetNode->SetLongName((const char *)m_tArtNetParams.aLongName);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_NET)) {
		pArtNetNode->SetNetSwitch(m_tArtNetParams.nNet);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_SUBNET)) {
		pArtNetNode->SetSubnetSwitch(m_tArtNetParams.nSubnet);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_ID)) {
		pArtNetNode->SetManufacturerId(m_tArtNetParams.aManufacturerId);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_OEM_VALUE)) {
		pArtNetNode->SetOemValue(m_tArtNetParams.aOemValue);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_NETWORK_TIMEOUT)) {
		pArtNetNode->SetNetworkTimeout(m_tArtNetParams.nNetworkTimeout);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_MERGE_TIMEOUT)) {
		pArtNetNode->SetDisableMergeTimeout(m_tArtNetParams.bDisableMergeTimeout);
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE_A << i)) {
			pArtNetNode->SetMergeMode(i, (TMerge) m_tArtNetParams.nMergeModePort[i]);
		} else {
			pArtNetNode->SetMergeMode(i, (TMerge) m_tArtNetParams.nMergeMode);
		}

		if (isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL_A << i)) {
			pArtNetNode->SetPortProtocol(i, (TPortProtocol) m_tArtNetParams.nProtocolPort[i]);
		} else {
			pArtNetNode->SetPortProtocol(i, (TPortProtocol) m_tArtNetParams.nProtocol);
		}
	}
}

void ArtNetParams::Dump(void) {
#ifndef NDEBUG
	if (m_tArtNetParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if(isMaskSet(ARTNET_PARAMS_MASK_SHORT_NAME)) {
		printf(" %s=%s\n", PARAMS_NODE_SHORT_NAME, m_tArtNetParams.aShortName);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_LONG_NAME)) {
		printf(" %s=%s\n", PARAMS_NODE_LONG_NAME, m_tArtNetParams.aLongName);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_NET)) {
		printf(" %s=%d\n", PARAMS_NET, m_tArtNetParams.nNet);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_SUBNET)) {
		printf(" %s=%d\n", PARAMS_SUBNET, m_tArtNetParams.nSubnet);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE)) {
		printf(" %s=%d\n", PARAMS_UNIVERSE, m_tArtNetParams.nUniverse);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL)) {
		printf(" %s=%d [%s]\n", PARAMS_PROTOCOL, m_tArtNetParams.nProtocol, PROTOCOL2STRING(m_tArtNetParams.nProtocol));
	}

	if (isMaskSet(ARTNET_PARAMS_MASK_RDM)) {
		printf(" %s=%d [%s]\n", PARAMS_RDM, (int) m_tArtNetParams.bEnableRdm, BOOL2STRING(m_tArtNetParams.bEnableRdm));
		if (m_tArtNetParams.bEnableRdm) {
			printf("  %s=%d [%s]\n", PARAMS_RDM_DISCOVERY, (int) m_tArtNetParams.bRdmDiscovery, BOOL2STRING(m_tArtNetParams.bRdmDiscovery));
		}
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_TIMECODE)) {
		printf(" %s=%d [%s]\n", PARAMS_TIMECODE, (int) m_tArtNetParams.bUseTimeCode, BOOL2STRING(m_tArtNetParams.bUseTimeCode));
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_TIMESYNC)) {
		printf(" %s=%d [%s]\n", PARAMS_TIMESYNC, (int) m_tArtNetParams.bUseTimeCode, BOOL2STRING(m_tArtNetParams.bUseTimeSync));
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_OUTPUT)) {
		printf(" %s=%d\n", PARAMS_OUTPUT, (int) m_tArtNetParams.tOutputType);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_ID)) {
		printf(" %s=0x%.2X%.2X\n", PARAMS_NODE_MANUFACTURER_ID, m_tArtNetParams.aManufacturerId[0], m_tArtNetParams.aManufacturerId[1]);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_OEM_VALUE)) {
		printf(" %s=0x%.2X%.2X\n", PARAMS_NODE_OEM_VALUE, m_tArtNetParams.aOemValue[0], m_tArtNetParams.aOemValue[1]);
	}

	if (isMaskSet(ARTNET_PARAMS_MASK_NETWORK_TIMEOUT)) {
		printf(" %s=%d [%s]\n", PARAMS_NODE_NETWORK_DATA_LOSS_TIMEOUT, (int) m_tArtNetParams.nNetworkTimeout, (m_tArtNetParams.nNetworkTimeout == 0) ? "Disabled" : "");
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_MERGE_TIMEOUT)) {
		printf(" %s=%d [%s]\n", PARAMS_NODE_DISABLE_MERGE_TIMEOUT, (int) m_tArtNetParams.bDisableMergeTimeout, BOOL2STRING(m_tArtNetParams.bDisableMergeTimeout));
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE_A << i)) {
			printf(" %s=%d\n", PARAMS_UNIVERSE_PORT[i], m_tArtNetParams.nUniversePort[i]);
		}
	}

	if (isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE)) {
		printf(" %s=%s\n", PARAMS_MERGE_MODE, MERGEMODE2STRING(m_tArtNetParams.nMergeMode));
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE_A << i)) {
			printf(" %s=%s\n", PARAMS_MERGE_MODE_PORT[i], MERGEMODE2STRING(m_tArtNetParams.nMergeModePort[i]));
		}
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL_A << i)) {
			printf(" %s=%s\n", PARAMS_PROTOCOL_PORT[i], PROTOCOL2STRING(m_tArtNetParams.nProtocolPort[i]));
		}
	}
#endif
}

uint16_t ArtNetParams::HexUint16(const char *s) const {
	uint16_t ret = 0;
	uint8_t nibble;
	unsigned i = 0;

	while ((*s != '\0') && (i++ < 4)) {
		const char d = *s;

		if (isxdigit((int) d) == 0) {
			break;
		}

		nibble = d > '9' ? ((uint8_t) d | (uint8_t) 0x20) - (uint8_t) 'a' + (uint8_t) 10 : (uint8_t) (d - '0');
		ret = (ret << 4) | nibble;
		s++;
	}

	return ret;
}

void ArtNetParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((ArtNetParams *) p)->callbackFunction(s);
}

bool ArtNetParams::isMaskSet(uint32_t nMask) const {
	return (m_tArtNetParams.nSetList & nMask) == nMask;
}
