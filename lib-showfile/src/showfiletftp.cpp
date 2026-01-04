/**
 * @file showfiletftp.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>

#include "showfiletftp.h"
#include "showfile.h"

 #include "firmware/debug/debug_debug.h"

void ShowFileTFTP::Exit()
{
    DEBUG_ENTRY();

    ShowFile::Instance().EnableTFTP(false);

    DEBUG_EXIT();
}

bool ShowFileTFTP::FileOpen(const char* file_name, [[maybe_unused]] tftp::Mode mode)
{
    DEBUG_PRINTF("file_name=%s, mode=%d", file_name, static_cast<int>(mode));

    int32_t show_file_number;
    if (!showfile::FilenameCheck(file_name, show_file_number))
    {
        DEBUG_EXIT();
        return false;
    }

    file_ = fopen(file_name, "r");
    return (file_ != nullptr);
}

bool ShowFileTFTP::FileCreate(const char* file_name, [[maybe_unused]] tftp::Mode mode)
{
    DEBUG_PRINTF("file_name=%s, mode=%d", file_name, static_cast<int>(mode));

    int32_t show_file_number;
    if (!showfile::FilenameCheck(file_name, show_file_number))
    {
        DEBUG_EXIT();
        return false;
    }

    file_ = fopen(file_name, "w+");
    return (file_ != nullptr);
}
