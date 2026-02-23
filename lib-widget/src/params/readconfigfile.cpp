/**
 * @file readconfigfile.cpp
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cstring>
#include <cassert>

#include "params/readconfigfile.h"
#include "firmware/debug/debug_dump.h"
 #include "firmware/debug/debug_debug.h"

// TODO Check this with GD32 64->128
static constexpr auto kMaxLineLength = 128; // Including '\0'

ReadConfigFile::ReadConfigFile(CallbackFunctionPtr callBack, void* p)
{
    assert(callBack != nullptr);
    assert(p != nullptr);

    callback_ = callBack;
    m_p = p;
}

ReadConfigFile::~ReadConfigFile()
{
    callback_ = nullptr;
    m_p = nullptr;
}

#if !defined(DISABLE_FS)
bool ReadConfigFile::Read(const char* pFileName)
{
    assert(pFileName != nullptr);

    char buffer[kMaxLineLength];

    FILE* fp = fopen(pFileName, "r");

    if (fp != nullptr)
    {
        for (;;)
        {
            if (fgets(buffer, static_cast<int>(sizeof(buffer)) - 1, fp) != buffer)
            {
                break; // Error or end of file
            }

            if (buffer[0] >= 'a')
            {
                char* q = buffer;

                for (unsigned i = 0; (i < sizeof(buffer) - 1) && (*q != '\0'); i++)
                {
                    if ((*q == '\r') || (*q == '\n'))
                    {
                        *q = '\0';
                    }
                    q++;
                }

                callback_(m_p, buffer);
            }
        }

        fclose(fp);
    }
    else
    {
        return false;
    }

    return true;
}
#endif

void ReadConfigFile::Read(const char* pBuffer, uint32_t length)
{
    DEBUG_ENTRY();

    assert(pBuffer != nullptr);
    assert(length != 0);

    const auto* src = const_cast<char*>(pBuffer);
    char buffer[kMaxLineLength];
    buffer[0] = '\n';

    debug::Dump(pBuffer, length);

    while (length != 0)
    {
        char* pLine = &buffer[0];

        while ((length != 0) && (*src != '\r') && (*src != '\n'))
        {
            *pLine++ = *src++;

            if ((pLine - buffer) >= kMaxLineLength)
            {
                DEBUG_PRINTF("%128s", &buffer[0]);
                assert(0);
                return;
            }

            length--;
        }

        while ((length != 0) && ((*src == '\r') || (*src == '\n')))
        {
            src++;
            length--;
        }

        if (buffer[0] >= '0')
        {
            *pLine = '\0';
            DEBUG_PUTS(&buffer[0]);
            callback_(m_p, &buffer[0]);
        }
    }

    DEBUG_EXIT();
}
