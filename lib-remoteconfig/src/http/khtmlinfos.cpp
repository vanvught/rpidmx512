/**
 * @file khtmlinfos.cpp
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

#include <cstddef>

#include "common/utils/utils_hash.h"
#include "http/html_infos.h"

namespace html
{
#define ENTRY(filename_literal, label) MakeHtmlInfo(filename_literal, Fnv1a32(filename_literal, static_cast<uint8_t>(sizeof(filename_literal) - 1)), label)

constexpr Info kHtmlInfos[] =
{
	ENTRY("/", "index.html"),
	ENTRY("/dmx", "dmx.html"),
#if defined (RDM_CONTROLLER)  
	ENTRY("/rdm", "rdm.html"),
#endif
#if defined (ENABLE_PHY_SWITCH)	
	ENTRY("/dsa", "dsa.html"), 
#endif	
#if !defined (DISABLE_RTC)	
	ENTRY("/rtc", "rtc.html"),
#endif	
#if defined (NODE_SHOWFILE)	
	ENTRY("/showfile", "showfile.html"),
#endif	
	ENTRY("/time", "time.html"),
#if defined(CONFIG_HTTPD_ENABLE_UPLOAD)	
	ENTRY("/upload_firmware", "upload_firmware.html")
#endif	
};

constexpr size_t kHtmlInfosSize = sizeof(kHtmlInfos) / sizeof(kHtmlInfos[0]);

static_assert(HasUniqueHashes(kHtmlInfos, kHtmlInfosSize), "Duplicate filename hashes detected in kHtmlInfosSize!");
} // namespace html