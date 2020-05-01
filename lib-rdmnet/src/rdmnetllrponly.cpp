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
#include <assert.h>

#include "rdmnetllrponly.h"

#include "rdmnetdevice.h"
#include "lightset.h"
#include "rdmidentify.h"
#include "rdmpersonality.h"

#include "debug.h"

#define DESCRIPTION		"RDMNet LLRP Only"
#define LABEL			DESCRIPTION
#define LABEL_LENGTH	(sizeof(LABEL) - 1)

RDMNetLLRPOnly::RDMNetLLRPOnly(void):
	m_RDMNetDevice(new RDMPersonality(DESCRIPTION, LightSet::Get()->GetDmxFootprint()))
{
	DEBUG_ENTRY

	DEBUG_EXIT
}

RDMNetLLRPOnly::~RDMNetLLRPOnly(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void RDMNetLLRPOnly::Init(void) {
	DEBUG_ENTRY

	m_RDMNetDevice.SetLabel(0, LABEL, LABEL_LENGTH);
	m_RDMNetDevice.Init();

	DEBUG_EXIT
}

void RDMNetLLRPOnly::Start(void) {
	DEBUG_ENTRY

	m_RDMNetDevice.Start();

	DEBUG_EXIT
}

void RDMNetLLRPOnly::Stop(void) {
	DEBUG_ENTRY

	m_RDMNetDevice.Stop();

	DEBUG_EXIT
}

void RDMNetLLRPOnly::SetMode(TRdmIdentifyMode nMode) {
	DEBUG_ENTRY

	DEBUG_EXIT
}
