/**
 * @file showfile_filename.cpp
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cctype>
#include <cassert>

#include "formats/showfileformatola.h"
#include "showfile.h"

 #include "firmware/debug/debug_debug.h"

namespace showfile
{
bool FilenameCopyto(char* show_file_name, uint32_t length, int32_t show_file_number)
{
    assert(length == showfile::kFileNameLength + 1);

    if (show_file_number <= showfile::kFileMaxNumber)
    {
        snprintf(show_file_name, length, "show%.2u.txt", static_cast<unsigned int>(show_file_number));
        return true;
    }

    return false;
}

bool FilenameCheck(const char* show_file_name, int32_t& show_file_number)
{
    DEBUG_PRINTF("show_file_name=[%s]", show_file_name);

    if ((show_file_name == nullptr) || (strlen(show_file_name) != showfile::kFileNameLength))
    {
        DEBUG_EXIT();
        return false;
    }

    if (memcmp(show_file_name, SHOWFILE_PREFIX, sizeof(SHOWFILE_PREFIX) - 1) != 0)
    {
        DEBUG_EXIT();
        return false;
    }

    if (memcmp(&show_file_name[showfile::kFileNameLength - sizeof(SHOWFILE_SUFFIX) + 1], SHOWFILE_SUFFIX, sizeof(SHOWFILE_SUFFIX) - 1) != 0)
    {
        DEBUG_EXIT();
        return false;
    }

    char digit = show_file_name[sizeof(SHOWFILE_PREFIX) - 1];
    DEBUG_PRINTF("digit=%c", digit);

    if (!isdigit(digit))
    {
        DEBUG_EXIT();
        return false;
    }

    show_file_number = 10 * (digit - '0');

    digit = show_file_name[sizeof(SHOWFILE_PREFIX)];
    DEBUG_PRINTF("digit=%c", digit);

    if (!isdigit(digit))
    {
        DEBUG_EXIT();
        return false;
    }

    show_file_number += digit - '0';

    DEBUG_EXIT();
    return true;
}

} // namespace showfile
