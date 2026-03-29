/**
 * @file get_file_content.cpp
 *
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "firmware/debug/debug_debug.h"

#if defined(CONFIG_HTTP_CONTENT_FS)
#include <cstdio>

#include "httpd/httpd.h"

static constexpr char kSupportedExtensions[static_cast<int>(http::ContentTypes::kNotDefined)][8] = 
{
	"html", 
	"css", 
	"js", 
	"json", 
	"bin"
};

static char s_static_content[4096];

static http::ContentTypes GetContentType(const char* file_name)
{
    DEBUG_ENTRY();

    for (int i = 0; i < static_cast<int>(http::ContentTypes::kNotDefined); i++)
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
                    return static_cast<http::ContentTypes>(i);
                }
            }
        }
    }

    DEBUG_EXIT();
    return http::ContentTypes::kNotDefined;
}

uint32_t GetFileContentFromFile(const char* file_name, char* dst, http::ContentTypes& content_type)
{
    DEBUG_PUTS(file_name);

    auto* file = fopen(file_name, "r");

    if (file == nullptr)
    {
        DEBUG_EXIT();
        return 0;
    }

    content_type = GetContentType(file_name);

    if (content_type == http::ContentTypes::kNotDefined)
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
        if ((p - dst) == sizeof(s_static_content))
        {
            DEBUG_PUTS("File too long");
            break;
        }
    }

    fclose(file);

    DEBUG_PRINTF("%s -> %d", file_name, static_cast<int>(p - dst));
    return static_cast<uint32_t>(p - dst);
}

const char* GetFileContent(const char* file_name, uint32_t& size, http::ContentTypes& content_type)
{
    DEBUG_ENTRY();
    DEBUG_PUTS(file_name);

    size = GetFileContentFromFile(file_name, s_static_content, content_type);

    if (size != 0)
    {
        return s_static_content;
    }

    DEBUG_EXIT();
    return nullptr;
}
#else
#include "../http/content/content.h"
#include "common/utils/utils_hash.h"

const char* GetFileContent(const char* file_name, uint32_t& size, http::ContentTypes& content_type)
{
    DEBUG_ENTRY();
    DEBUG_PUTS(file_name);
	
	const auto kHash = Fnv1a32Runtime(file_name, strlen(file_name));

    for (auto& content : kHttpContent)
    {
        if (kHash == content.hash)
        {
            size = content.content_length;
            content_type = content.content_type;
            return content.content;
        }
    }

    size = 0;
    content_type = http::ContentTypes::kNotDefined;

    DEBUG_EXIT();
    return nullptr;
}
#endif
