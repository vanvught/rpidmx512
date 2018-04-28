/*
 * rdmslotdescription.cpp
 *
 *  Created on: Jan 15, 2018
 *      Author: pi
 */

#include <stdint.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#if defined (BARE_METAL)
 #include "util.h"
#elif defined(__circle__)
 #include "circle/util.h"
#else
  #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "rdmslotinfo.h"

#include "rdm_e120.h"
#include "rdmslot.h"

// Appendix C: Slot Info (Normative)

#define TABLE_C1_SIZE	9
#define TABLE_C2_SIZE	40

struct TTableC1 {
	const TRdmSlotType nId;
	const char *pDescription;
}static const s_tTableC1[TABLE_C1_SIZE] ALIGNED = {
		{ ST_PRIMARY, "Slot directly controls parameter (represents Coarse for 16-bit parameters)" },
		{ ST_SEC_FINE, "Fine, for 16-bit parameters" },
		{ ST_SEC_TIMING, "Slot sets timing value for associated parameter" },
		{ ST_SEC_SPEED, "Slot sets speed/velocity for associated parameter" },
		{ ST_SEC_CONTROL, "Slot provides control/mode info for parameter" },
		{ ST_SEC_INDEX, "Slot sets index position for associated parameter" },
		{ ST_SEC_ROTATION, "Slot sets rotation speed for associated parameter" },
		{ ST_SEC_INDEX_ROTATE, "Combined index/rotation control" },
		{ ST_SEC_UNDEFINED, "Undefined secondary type" } };

struct TTableC2 {
	const TRdmSlotDefinition nId;
	const char *pDescription;
}static const s_tTableC2[TABLE_C2_SIZE] ALIGNED = {
		// Intensity Functions
		{ SD_INTENSITY, "Intensity" },
		{ SD_INTENSITY_MASTER, "Intensity Master" },
		// Movement Functions
		{ SD_PAN, "Pan" },
		{ SD_TILT, "Tilt" },
		// Color Functions
		{ SD_COLOR_WHEEL, "Color Wheel" },
		{ SD_COLOR_SUB_CYAN, "Subtractive Color Mixer – Cyan/Blue" },
		{ SD_COLOR_SUB_YELLOW, "Subtractive Color Mixer – Yellow/Amber" },
		{ SD_COLOR_SUB_MAGENTA, "Subtractive Color Mixer - Magenta" },
		{ SD_COLOR_ADD_RED, "Additive Color Mixer - Red" },
		{ SD_COLOR_ADD_GREEN, "Additive Color Mixer - Green" },
		{ SD_COLOR_ADD_BLUE, "Additive Color Mixer - Blue" },
		{ SD_COLOR_CORRECTION, "Color Temperature Correction" },
		{ SD_COLOR_ADD_AMBER, "Additive Color Mixer - Amber" },
		{ SD_COLOR_ADD_WHITE, "Additive Color Mixer - White" },
		{ SD_COLOR_ADD_WARM_WHITE, "Additive Color Mixer - Warm White" },
		{ SD_COLOR_ADD_COOL_WHITE, "Additive Color Mixer - Cool White" },
		{ SD_COLOR_SUB_UV, "Subtractive Color Mixer - UV" },
		{ SD_COLOR_HUE, "Hue" },
		{ SD_COLOR_SATURATION, "Saturation" },
		// Image Functions
		{ SD_STATIC_GOBO_WHEEL, "Static gobo wheel" },
		{ SD_ROTO_GOBO_WHEEL, "Rotating gobo wheel" },
		{ SD_PRISM_WHEEL, "Prism wheel" },
		{ SD_EFFECTS_WHEEL, "Effects wheel" },
		// Beam Functions
		{ SD_BEAM_SIZE_IRIS, "Beam size iris" },
		{ SD_EDGE, "Edge/Lens focus" },
		{ SD_FROST, "Frost/Diffusion" },
		{ SD_STROBE, "Strobe/Shutter" },
		{ SD_ZOOM, "Zoom lens" },
		{ SD_FRAMING_SHUTTER, "Framing shutter" },
		{ SD_SHUTTER_ROTATE, "Framing shutter rotation" },
		{ SD_DOUSER, "Douser" },
		{ SD_BARN_DOOR, "Barn Door" },
		// Control Functions
		{ SD_LAMP_CONTROL, "Lamp control functions" },
		{ SD_FIXTURE_CONTROL, "Fixture control channel" },
		//
		{ SD_MACRO, "Macro control" },
		{ SD_POWER_CONTROL, "Relay or power control" },
		{ SD_FAN_CONTROL, "Fan control" },
		{ SD_HEATER_CONTROL, "Heater control" },
		{ SD_FOUNTAIN_CONTROL, "Fountain water pump control " },
		// Undefined
		{ SD_UNDEFINED, "No definition" } };

RDMSlotInfo::RDMSlotInfo(void) {
}

RDMSlotInfo::~RDMSlotInfo(void) {
}

const char* RDMSlotInfo::GetTypeText(uint8_t nId, uint8_t& nLength) {

	for (uint8_t i = 0; i < TABLE_C1_SIZE; i++) {
		if (nId == s_tTableC1[i].nId) {
			nLength = strlen(s_tTableC1[i].pDescription);
			return s_tTableC1[i].pDescription;
		}
	}

	nLength = strlen(s_tTableC1[TABLE_C1_SIZE - 1].pDescription);
	return s_tTableC1[TABLE_C1_SIZE - 1].pDescription;
}

const char* RDMSlotInfo::GetCategoryText(uint16_t nId, uint8_t& nLength) {
	int nIndex = bsearch(nId);

	if (nIndex < 0) {
		nLength = 0;
		return 0;
	}

	nLength = strlen(s_tTableC2[nIndex].pDescription);
	return s_tTableC2[nIndex].pDescription;
}

int RDMSlotInfo::bsearch(uint16_t nKey) {
	int nLow = 0;
	int nHigh = TABLE_C2_SIZE;

	while (nLow <= nHigh) {
		const int nMid = nLow + ((nHigh - nLow) / 2);
		const int nMidValue = (const int) s_tTableC2[nMid].nId;

		if (nMidValue < nKey) {
			nLow = nMid + 1;
		} else if (nMidValue > nKey) {
			nHigh = nMid - 1;
		} else {
			return nMid; // nKey found
		}
	}

	return -1; // nKey not found
}
