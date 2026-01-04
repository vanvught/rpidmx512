/**
 * @file json_status_showfile.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "showfile.h"
#include "json/showfileparamsconst.h"

namespace json::status {
uint32_t ShowFile(char* out_buffer, uint32_t out_buffer_size)
{
    const auto kStatus = ShowFile::Instance().GetStatus();
    assert(kStatus != ::showfile::Status::kUndefined);

    const auto kLength =
        static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size, "{\"mode\":\"%s\",\"%s\":\"%u\",\"status\":\"%s\",\"%s\":\"%s\"}",
        ShowFile::Instance().GetMode() == ::showfile::Mode::kRecorder ? "Recorder" : "Player", ShowFileParamsConst::kShow.name,
        static_cast<unsigned int>(ShowFile::Instance().GetShowFileCurrent()), ::showfile::kStatus[static_cast<int>(kStatus)],
        ShowFileParamsConst::kOptionLoop.name, ShowFile::Instance().GetDoLoop() ? "1" : "0"));
    return kLength;
}
}  // namespace json::status
