/**
 * @file json_status.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "showfile.h"
#include "showfileconst.h"
#include "showfileparamsconst.h"
#include "showfiledisplay.h"

#include "readconfigfile.h"
#include "sscan.h"

static void staticCallbackFunction([[maybe_unused]] void *p, const char *s) {
	assert(p == nullptr);
	assert(s != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(s, ShowFileParamsConst::SHOW, nValue8) == Sscan::OK) {
		if (nValue8 <= showfile::FILE_MAX_NUMBER) {
			ShowFile::Get()->SetPlayerShowFileCurrent(nValue8);
		}
		return;
	}

#if !defined (CONFIG_SHOWFILE_DISABLE_RECORD)
	if (Sscan::Uint8(s, "recorder", nValue8) == Sscan::OK) {
		if (nValue8 <= showfile::FILE_MAX_NUMBER) {
			ShowFile::Get()->SetRecorderShowFileCurrent(nValue8);
		}
		return;
	}
#endif

	if (Sscan::Uint8(s, ShowFileParamsConst::OPTION_LOOP, nValue8) == Sscan::OK) {
		ShowFile::Get()->DoLoop(nValue8 != 0);
		return;
	}

	char action[8];
	uint32_t nLength = sizeof(action) - 1;

	if (Sscan::Char(s, "status" , action, nLength) == Sscan::OK) {
		if (strncmp(action, "play", nLength) == 0) {
			ShowFile::Get()->Play();
			return;
		}

		if (strncmp(action, "stop", nLength) == 0) {
			ShowFile::Get()->Stop();
			return;
		}

		if (strncmp(action, "resume", nLength) == 0) {
			ShowFile::Get()->Resume();
			return;
		}

#if !defined (CONFIG_SHOWFILE_DISABLE_RECORD)
		if (strncmp(action, "record", nLength) == 0) {
			ShowFile::Get()->Record();
			return;
		}
#endif

		return;
	}
}

namespace remoteconfig {
namespace showfile {
uint32_t json_get_status(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto status = ShowFile::Get()->GetStatus();
	assert(status != ::showfile::Status::UNDEFINED);

	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
						"{\"mode\":\"%s\",\"%s\":\"%u\",\"status\":\"%s\",\"%s\":\"%s\"}",
						ShowFile::Get()->GetMode() == ::showfile::Mode::RECORDER ? "Recorder" : "Player",
						ShowFileParamsConst::SHOW,
						static_cast<unsigned int>(ShowFile::Get()->GetShowFileCurrent()),
						::showfile::STATUS[static_cast<int>(status)],
						ShowFileParamsConst::OPTION_LOOP,
						ShowFile::Get()->GetDoLoop() ? "1" : "0"));
	return nLength;
}

void json_set_status(const char *pBuffer, const uint32_t nBufferSize) {
	ReadConfigFile config(staticCallbackFunction, nullptr);
	config.Read(pBuffer, nBufferSize);
	::showfile::display_status();
}
}  // namespace showfile
}  // namespace remoteconfig
