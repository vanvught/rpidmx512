/**
 * @file get_file_content.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_HTTPD)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>

#include "httpd/httpd.h"

 #include "firmware/debug/debug_debug.h"

#if defined(CONFIG_HTTP_CONTENT_FS)
#include <cstdio>

static constexpr char kSupportedExtensions[static_cast<int>(http::contentTypes::NOT_DEFINED)][8] = {"html", "css", "js", "json", "bin"};

static http::contentTypes GetContentType(const char* file_name)
{
	DEBUG_ENTRY();
	
    for (int i = 0; i < static_cast<int>(http::contentTypes::NOT_DEFINED); i++)
    {
        const auto kL = strlen(file_name);
        const auto kE = strlen(kSupportedExtensions[i]);

        if (kL > (kE + 2))
        {
            if (file_name[kL - kE - 1] == '.')
            {
                if (strcmp(&file_name[kL - kE], kSupportedExtensions[i]) == 0)
                {
					DEBUG_EXIT();
                    return static_cast<http::contentTypes>(i);
                }
            }
        }
    }

DEBUG_EXIT();
    return http::contentTypes::NOT_DEFINED;
}

uint32_t GetFileContent(const char* file_name, char* dst, http::contentTypes& content_type)
{
	DEBUG_PUTS(file_name);
	
    auto* file = fopen(file_name, "r");

    if (file == nullptr)
    {
        DEBUG_EXIT();
        return 0;
    }

    content_type = GetContentType(file_name);

    if (content_type == http::contentTypes::NOT_DEFINED)
    {
        DEBUG_EXIT();
        fclose(file);
        return 0;
    }

    auto do_remove_white_spaces = true;
    auto* p = dst;
    int c;

    while ((c = fgetc(file)) != EOF)
    {
        if (do_remove_white_spaces)
        {
            if (c <= ' ')
            {
                continue;
            }
            else
            {
                do_remove_white_spaces = false;
            }
        }
        else
        {
            if (c == '\n')
            {
                do_remove_white_spaces = true;
            }
        }
        *p++ = c;
        if ((p - dst) == httpd::kBufsize)
        {
            DEBUG_PUTS("File too long");
            break;
        }
    }

    fclose(file);

    DEBUG_PRINTF("%s -> %d", file_name, static_cast<int>(p - dst));
    return static_cast<uint32_t>(p - dst);
}

static char s_static_content[httpd::kBufsize];

const char* GetFileContent(const char* file_name, uint32_t& size, http::contentTypes& content_type)
{
    DEBUG_ENTRY();
    DEBUG_PUTS(file_name);

    size = GetFileContent(file_name, s_static_content, content_type);

    if (size != 0)
    {
        return s_static_content;
    }

    DEBUG_EXIT();
    return nullptr;
}
#else
#include "../http/content/content.h"
const char* GetFileContent(const char* file_name, uint32_t& size, http::contentTypes& contentType)
{
    DEBUG_ENTRY();
    DEBUG_PUTS(file_name);

    for (auto& content : HttpContent)
    {
        if (strcmp(file_name, content.pFileName) == 0)
        {
            size = content.nContentLength;
            contentType = content.contentType;
            return content.pContent;
        }
    }

    size = 0;
    contentType = http::contentTypes::NOT_DEFINED;

    DEBUG_EXIT();
    return nullptr;
}
#endif
