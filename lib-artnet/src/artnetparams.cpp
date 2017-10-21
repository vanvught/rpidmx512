/**
 * @file artnetparams.cpp
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#if defined(__linux__) || defined (__CYGWIN__)
#define ALIGNED
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#else
#include "util.h"
#endif

#include "artnetparams.h"
#include "artnetnode.h"
#include "common.h"

#include "readconfigfile.h"
#include "sscan.h"

#define BOOL2STRING(b)	(b) ? "Yes" : "No"

#define SET_LONG_NAME_MASK	1<<0
#define SET_SHORT_NAME_MASK	1<<1
#define SET_NET_MASK		1<<2
#define SET_SUBNET_MASK		1<<3
#define SET_UNIVERSE_MASK	1<<4
#define SET_RDM_MASK		1<<5
#define SET_TIMECODE_MASK	1<<6
#define SET_TIMESYNC_MASK	1<<7
#define SET_OUTPUT_MASK		1<<8
#define SET_ID_MASK			1<<9
#define SET_OEM_VALUE_MASK	1<<10

static const char PARAMS_FILE_NAME[] ALIGNED = "artnet.txt";					///< Parameters file name
static const char PARAMS_NET[] ALIGNED = "net";									///<
static const char PARAMS_SUBNET[] ALIGNED = "subnet";							///<
static const char PARAMS_UNIVERSE[] ALIGNED = "universe";						///<
static const char PARAMS_OUTPUT[] ALIGNED = "output";							///< dmx {default}, spi, mon
static const char PARAMS_TIMECODE[] ALIGNED = "use_timecode";					///< Use the TimeCode call-back handler, 0 {default}
static const char PARAMS_TIMESYNC[] ALIGNED = "use_timesync";					///< Use the TimeSync call-back handler, 0 {default}
static const char PARAMS_RDM[] ALIGNED = "enable_rdm";							///< Enable RDM, 0 {default}
static const char PARAMS_RDM_DISCOVERY[] ALIGNED = "rdm_discovery_at_startup";	///< 0 {default}
static const char PARAMS_NODE_SHORT_NAME[] ALIGNED = "short_name";				///<
static const char PARAMS_NODE_LONG_NAME[] ALIGNED = "long_name";				///<
static const char PARAMS_NODE_MANUFACTURER_ID[] ALIGNED = "manufacturer_id";	///<
static const char PARAMS_NODE_OEM_VALUE[] ALIGNED = "oem_value";				///<

void ArtNetParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((ArtNetParams *) p)->callbackFunction(s);
}

void ArtNetParams::callbackFunction(const char *pLine) {
	char value[128];
	uint8_t len;
	uint8_t value8;

	if (Sscan::Uint8(pLine, PARAMS_TIMECODE, &value8) == 2) {
		if (value8 != 0) {
			m_bUseTimeCode = true;
			m_bSetList |= SET_TIMECODE_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_TIMESYNC, &value8) == 2) {
		if (value8 != 0) {
			m_bUseTimeSync = true;
			m_bSetList |= SET_TIMESYNC_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_RDM, &value8) == 2) {
		if (value8 != 0) {
			m_bEnableRdm = true;
			m_bSetList |= SET_RDM_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_RDM_DISCOVERY, &value8) == 2) {
		if (value8 != 0) {
			m_bRdmDiscovery = true;
		}
		return;
	}

	len = ARTNET_SHORT_NAME_LENGTH;
	if (Sscan::Char(pLine, PARAMS_NODE_SHORT_NAME, value, &len) == 2) {
		strncpy((char *)m_aShortName, value, len);
		m_bSetList |= SET_SHORT_NAME_MASK;
		return;
	}

	len = ARTNET_LONG_NAME_LENGTH;
	if (Sscan::Char(pLine, PARAMS_NODE_LONG_NAME, value, &len) == 2) {
		strncpy((char *)m_aLongName, value, len);
		m_bSetList |= SET_LONG_NAME_MASK;
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_OUTPUT, value, &len) == 2) {
		if (memcmp(value, "spi", 3) == 0) {
			m_tOutputType = OUTPUT_TYPE_SPI;
			m_bSetList |= SET_OUTPUT_MASK;
		} else if (memcmp(value, "mon", 3) == 0) {
			m_tOutputType = OUTPUT_TYPE_MONITOR;
			m_bSetList |= SET_OUTPUT_MASK;
		}
		return;
	}

	len = 4;
	if (Sscan::Char(pLine, PARAMS_NODE_MANUFACTURER_ID, value, &len) == 2) {
		if (len == 4) {
			const uint16_t v = HexUint16(value);
			m_aManufacturerId[0] = (uint8_t) (v >> 8);
			m_aManufacturerId[1] = (uint8_t) (v & 0xFF);
			m_bSetList |= SET_ID_MASK;
		}
		return;
	}

	len = 4;
	if (Sscan::Char(pLine, PARAMS_NODE_OEM_VALUE, value, &len) == 2) {
		if (len == 4) {
			const uint16_t v = HexUint16(value);
			m_aOemValue[0] = (uint8_t) (v >> 8);
			m_aOemValue[1] = (uint8_t) (v & 0xFF);
			m_bSetList |= SET_OEM_VALUE_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_NET, &value8) == 2) {
		m_nNet = value8;
		m_bSetList |= SET_NET_MASK;
	} else if (Sscan::Uint8(pLine, PARAMS_SUBNET, &value8) == 2) {
		m_nSubnet = value8;
		m_bSetList |= SET_SUBNET_MASK;
	} else if (Sscan::Uint8(pLine, PARAMS_UNIVERSE, &value8) == 2) {
		m_nUniverse = value8;
		m_bSetList |= SET_UNIVERSE_MASK;
	}

}

ArtNetParams::ArtNetParams(void): m_bSetList(0) {
	m_nNet = 0;
	m_nSubnet = 0;
	m_nUniverse = 0;
	m_tOutputType= OUTPUT_TYPE_DMX;
	m_bUseTimeCode = false;
	m_bUseTimeSync = false;
	m_bEnableRdm = false;
	m_bRdmDiscovery = false;

	memset(m_aShortName, 0, ARTNET_SHORT_NAME_LENGTH);
	memset(m_aLongName, 0, ARTNET_LONG_NAME_LENGTH);
}

ArtNetParams::~ArtNetParams(void) {
}

const tOutputType ArtNetParams::GetOutputType(void) {
	return m_tOutputType;
}

const uint8_t ArtNetParams::GetNet(void) {
	return m_nNet;
}

const uint8_t ArtNetParams::GetSubnet(void) {
	return m_nSubnet;
}

const uint8_t ArtNetParams::GetUniverse(void) {
	return m_nUniverse;
}

const bool ArtNetParams::IsUseTimeCode(void) {
	return m_bUseTimeCode;
}

const bool ArtNetParams::IsUseTimeSync(void) {
	return m_bUseTimeSync;
}

const bool ArtNetParams::IsRdm(void) {
	return m_bEnableRdm;
}

const bool ArtNetParams::IsRdmDiscovery(void) {
	return m_bRdmDiscovery;
}


const uint8_t *ArtNetParams::GetShortName(void) {
	return m_aShortName;
}

const uint8_t *ArtNetParams::GetLongName(void) {
	return m_aLongName;
}

const uint8_t *ArtNetParams::GetManufacturerId(void) {
	return m_aManufacturerId;
}

bool ArtNetParams::Load(void) {
	m_bSetList = 0;

	ReadConfigFile configfile(ArtNetParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void ArtNetParams::Set(ArtNetNode *pArtNetNode) {
	if (m_bSetList == 0) {
		return;
	}

	if(isMaskSet(SET_SHORT_NAME_MASK)) {
		pArtNetNode->SetShortName((const char *)m_aShortName);
	}

	if(isMaskSet(SET_LONG_NAME_MASK)) {
		pArtNetNode->SetLongName((const char *)m_aLongName);
	}

	if(isMaskSet(SET_NET_MASK)) {
		pArtNetNode->SetNetSwitch(m_nNet);
	}

	if(isMaskSet(SET_SUBNET_MASK)) {
		pArtNetNode->SetSubnetSwitch(m_nSubnet);
	}

	if(isMaskSet(SET_ID_MASK)) {
		pArtNetNode->SetManufacturerId(m_aManufacturerId);
	}

	if(isMaskSet(SET_OEM_VALUE_MASK)) {
		pArtNetNode->SetOemValue(m_aOemValue);
	}
}

#if defined(__linux__) || defined (__CYGWIN__)
void ArtNetParams::Dump(void) {
	if (m_bSetList == 0) {
		return;
	}

	printf("Node params \'%s\':\n", PARAMS_FILE_NAME);

	if(isMaskSet(SET_SHORT_NAME_MASK)) {
		printf(" Short name : [%s]\n", m_aShortName);
	}

	if(isMaskSet(SET_LONG_NAME_MASK)) {
		printf(" Long name : [%s]\n", m_aLongName);
	}

	if(isMaskSet(SET_NET_MASK)) {
		printf(" Net : %d\n", m_nNet);
	}

	if(isMaskSet(SET_SUBNET_MASK)) {
		printf(" Sub-Net : %d\n", m_nSubnet);
	}

	if(isMaskSet(SET_UNIVERSE_MASK)) {
		printf(" Universe : %d\n", m_nUniverse);
	}

	if (isMaskSet(SET_RDM_MASK)) {
		printf(" RDM Enabled : %s\n", BOOL2STRING(m_bEnableRdm));
		if (m_bEnableRdm) {
			printf("    Discovery : %s\n", BOOL2STRING(m_bRdmDiscovery));
		}
	}

	if(isMaskSet(SET_TIMECODE_MASK)) {
		printf(" TimeCode enabled : %s\n", BOOL2STRING(m_bUseTimeCode));
	}

	if(isMaskSet(SET_TIMESYNC_MASK)) {
		printf(" TimeSync enabled : %s\n", BOOL2STRING(m_bUseTimeSync));
	}

	if(isMaskSet(SET_OUTPUT_MASK)) {
		printf(" Output : %d\n", (int) m_tOutputType);
	}

	if(isMaskSet(SET_ID_MASK)) {
		printf(" Manufacturer ID (ESTA) : 0x%.2X%.2X\n", m_aManufacturerId[0], m_aManufacturerId[1]);
	}

	if(isMaskSet(SET_OEM_VALUE_MASK)) {
		printf(" OEM Value : 0x%.2X%.2X\n", m_aOemValue[0], m_aOemValue[1]);
	}
}
#endif

bool ArtNetParams::isMaskSet(uint16_t mask) {
	return (m_bSetList & mask) == mask;
}

const uint16_t ArtNetParams::HexUint16(const char *s) {
	uint16_t ret = 0;
	uint8_t nibble;
	uint8_t i = 0;

	while ((*s != '\0') && (i++ < 4)) {
		char d = *s;

		if (isxdigit((int) d) == 0) {
			break;
		}

		nibble = d > '9' ? ((uint8_t) d | (uint8_t) 0x20) - (uint8_t) 'a' + (uint8_t) 10 : (uint8_t) (d - '0');
		ret = (ret << 4) | nibble;
		s++;
	}

	return ret;
}
