/**
 * @file rdmnetllrponly.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "rdmnetllrponly.h"

#include "rdmnetdevice.h"
#include "lightset.h"
#include "rdmidentify.h"
#include "rdmpersonality.h"
#include "rdm.h"

#include "debug.h"

static constexpr char LABEL[] = "RDMNet LLRP Only";
static constexpr auto LABEL_LENGTH = sizeof(LABEL) - 1;

RDMNetLLRPOnly::RDMNetLLRPOnly(const char *pLabel):
	m_pLabel(const_cast<char*>(pLabel)),
	m_RDMNetDevice(new RDMPersonality(LABEL, LightSet::Get()->GetDmxFootprint())) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

RDMNetLLRPOnly::~RDMNetLLRPOnly() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void RDMNetLLRPOnly::Init() {
	DEBUG_ENTRY

	if (m_pLabel == nullptr) {
		m_RDMNetDevice.SetLabel(RDM_ROOT_DEVICE, LABEL, LABEL_LENGTH);
	} else {
		m_RDMNetDevice.SetLabel(RDM_ROOT_DEVICE, m_pLabel, strlen(m_pLabel));
	}
	m_RDMNetDevice.Init();

	DEBUG_EXIT
}

void RDMNetLLRPOnly::Start() {
	DEBUG_ENTRY

	m_RDMNetDevice.Start();

	DEBUG_EXIT
}

void RDMNetLLRPOnly::Stop() {
	DEBUG_ENTRY

	m_RDMNetDevice.Stop();

	DEBUG_EXIT
}

void RDMNetLLRPOnly::SetMode(__attribute__((unused)) TRdmIdentifyMode nMode) {
	DEBUG_ENTRY

	DEBUG_EXIT
}
