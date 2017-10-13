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

#if defined(__linux__) || defined (__CYGWIN__)
#define ALIGNED
#include <stdio.h>
#include <string.h>
#else
#include "util.h"
#endif

#include "artnetparams.h"
#include "common.h"

#include "read_config_file.h"
#include "sscan.h"

#define BOOL2STRING(b)	(b) ? "Yes" : "No"

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

static uint8_t ArtNetParamsNet ALIGNED;
static uint8_t ArtNetParamsSubnet ALIGNED;
static uint8_t ArtNetParamsUniverse ALIGNED;
static _output_type ArtNetParamsOutputType ALIGNED;
static bool ArtNetParamsUseTimeCode;
static bool ArtNetParamsUseTimeSync;
static bool ArtNetParamsEnableRdm;
static bool ArtNetParamsRdmDiscovery;
static uint8_t ArtNetParamsShortName[ARTNET_SHORT_NAME_LENGTH] ALIGNED;
static uint8_t ArtNetParamsLongName[ARTNET_LONG_NAME_LENGTH] ALIGNED;

static void process_line_read(const char *line) {
	char value[128];
	uint8_t len;
	uint8_t value8;

	if (sscan_uint8_t(line, PARAMS_TIMECODE, &value8) == 2) {
		if (value8 != 0) {
			ArtNetParamsUseTimeCode = true;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_TIMESYNC, &value8) == 2) {
		if (value8 != 0) {
			ArtNetParamsUseTimeSync = true;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_RDM, &value8) == 2) {
		if (value8 != 0) {
			ArtNetParamsEnableRdm = true;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_RDM_DISCOVERY, &value8) == 2) {
		if (value8 != 0) {
			ArtNetParamsRdmDiscovery = true;
		}
		return;
	}

	len = ARTNET_SHORT_NAME_LENGTH;
	if (sscan_char_p(line, PARAMS_NODE_SHORT_NAME, value, &len) == 2) {
		strncpy((char *)ArtNetParamsShortName, value, len);
	}

	len = ARTNET_LONG_NAME_LENGTH;
	if (sscan_char_p(line, PARAMS_NODE_LONG_NAME, value, &len) == 2) {
		strncpy((char *)ArtNetParamsLongName, value, len);
	}

	len = 3;
	if (sscan_char_p(line, PARAMS_OUTPUT, value, &len) == 2) {
		if (memcmp(value, "spi", 3) == 0) {
			ArtNetParamsOutputType = OUTPUT_TYPE_SPI;
		} else if (memcmp(value, "mon", 3) == 0) {
			ArtNetParamsOutputType = OUTPUT_TYPE_MONITOR;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_NET, &value8) == 2) {
		ArtNetParamsNet = value8;
	} else if (sscan_uint8_t(line, PARAMS_SUBNET, &value8) == 2) {
		ArtNetParamsSubnet = value8;
	} else if (sscan_uint8_t(line, PARAMS_UNIVERSE, &value8) == 2) {
		ArtNetParamsUniverse = value8;
	}

}

ArtNetParams::ArtNetParams(void) {
	ArtNetParamsNet = 0;
	ArtNetParamsSubnet = 0;
	ArtNetParamsUniverse = 0;
	ArtNetParamsOutputType= OUTPUT_TYPE_DMX;
	ArtNetParamsUseTimeCode = false;
	ArtNetParamsUseTimeSync = false;
	ArtNetParamsEnableRdm = false;
	ArtNetParamsRdmDiscovery = false;

	memset(ArtNetParamsShortName, 0, ARTNET_SHORT_NAME_LENGTH);
	memset(ArtNetParamsLongName, 0, ARTNET_LONG_NAME_LENGTH);
}

ArtNetParams::~ArtNetParams(void) {
}

const _output_type ArtNetParams::GetOutputType(void) {
	return ArtNetParamsOutputType;
}

const uint8_t ArtNetParams::GetNet(void) {
	return ArtNetParamsNet;
}

const uint8_t ArtNetParams::GetSubnet(void) {
	return ArtNetParamsSubnet;
}

const uint8_t ArtNetParams::GetUniverse(void) {
	return ArtNetParamsUniverse;
}

const bool ArtNetParams::IsUseTimeCode(void) {
	return ArtNetParamsUseTimeCode;
}

const bool ArtNetParams::IsUseTimeSync(void) {
	return ArtNetParamsUseTimeSync;
}

const bool ArtNetParams::IsRdm(void) {
	return ArtNetParamsEnableRdm;
}

const bool ArtNetParams::IsRdmDiscovery(void) {
	return ArtNetParamsRdmDiscovery;
}


const uint8_t *ArtNetParams::GetShortName(void) {
	return ArtNetParamsShortName;
}

const uint8_t *ArtNetParams::GetLongName(void) {
	return ArtNetParamsLongName;
}

bool ArtNetParams::Load(void) {
	return read_config_file(PARAMS_FILE_NAME, &process_line_read);
}

#if defined(__linux__) || defined (__CYGWIN__)
void ArtNetParams::Dump(void) {
	printf("Node params (\'%s\'):\n", PARAMS_FILE_NAME);
	printf(" Short name : [%s]\n", ArtNetParamsShortName);
	printf(" Long name : [%s]\n", ArtNetParamsLongName);
	printf(" Net : %d\n", ArtNetParamsNet);
	printf(" Sub-Net : %d\n", ArtNetParamsSubnet);
	printf(" Universe : %d\n", ArtNetParamsUniverse);
	printf(" RDM Enabled : %s\n", BOOL2STRING(ArtNetParamsEnableRdm));
	if (ArtNetParamsEnableRdm) {
		printf("    Discovery : %s\n", BOOL2STRING(ArtNetParamsRdmDiscovery));
	}
	printf(" TimeCode enabled : %s\n", BOOL2STRING(ArtNetParamsUseTimeCode));
	printf(" TimeSync enabled : %s\n", BOOL2STRING(ArtNetParamsUseTimeSync));
	printf(" Output : %d\n", (int) ArtNetParamsOutputType);
}
#endif
