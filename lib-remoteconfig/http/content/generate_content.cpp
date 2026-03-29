/**
 * @file generate_content.cpp
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

#undef NDEBUG

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <cassert>

#include "http/http.h"
#include "common/utils/utils_hash.h"

struct SupportedExtension
{
    const char* extension;
    http::ContentTypes content_type;
};

static constexpr SupportedExtension kSupportedExtensions[] = 
{
	{"html", http::ContentTypes::kTextHtml},
	{"css", http::ContentTypes::kTextCss},
	{"js", http::ContentTypes::kTextJs},
	{"json", http::ContentTypes::kApplicationJson}
};

static constexpr char kContentHeader[] =
    "\n"
    "struct FilesContent {\n"
	"\tuint32_t hash;\n"
    "\tconst char *file_name;\n"
    "\tconst char *content;\n"
    "\tuint32_t content_length;\n"
    "\thttp::ContentTypes content_type;\n"
    "};\n\n"
    "inline constexpr struct FilesContent kHttpContent[] = {\n";

static constexpr char kHaveDmxBegin[] = "#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))\n";
static constexpr char kHaveDmxEnd[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */\n";

static constexpr char kHaveRdmBegin[] = "#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)\n";
static constexpr char kHaveRdmEnd[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */\n";

static constexpr char kHavePixelBegin[] = "#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))\n";
static constexpr char kHavePixelEnd[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI)) */\n";

static constexpr char kHaveShowfileBegin[] = "#if defined (NODE_SHOWFILE)\n";
static constexpr char kHaveShowfileEnd[] = "#endif /* (NODE_SHOWFILE) */\n";

static constexpr char kHaveTimeBegin[] = "#if !defined (CONFIG_HTTP_HTML_NO_TIME)\n";
static constexpr char kHaveTimeEnd[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */\n";

static constexpr char kHaveRtcBegin[] = "#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)\n";
static constexpr char kHaveRtcEnd[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */\n";

struct FeatureGuard
{
    const char* match;
    const char* begin;
    const char* end;
};

static constexpr FeatureGuard kFeatureGuards[] = {
	{"dmx", kHaveDmxBegin, kHaveDmxEnd},
	{"rdm", kHaveRdmBegin, kHaveRdmEnd},
	{"pixel", kHavePixelBegin, kHavePixelEnd},
	{"showfile", kHaveShowfileBegin, kHaveShowfileEnd},
	{"time", kHaveTimeBegin, kHaveTimeEnd},
	{"rtc", kHaveRtcBegin, kHaveRtcEnd}
};

static FILE* file_content;
static FILE* file_includes;

static http::ContentTypes GetContentType(const char* file_name)
{
    assert(file_name != nullptr);

    const auto* dot = strrchr(file_name, '.');

    if ((dot == nullptr) || (*(dot + 1) == '\0'))
    {
        return http::ContentTypes::kNotDefined;
    }

    const auto* extension = dot + 1;

    for (const auto& entry : kSupportedExtensions)
    {
        if (strcmp(extension, entry.extension) == 0)
        {
            return entry.content_type;
        }
    }

    return http::ContentTypes::kNotDefined;
}

static bool HasMatchingFeature(const char* file_name, const char* match)
{
    assert(file_name != nullptr);
    assert(match != nullptr);

    return strstr(file_name, match) != nullptr;
}

static void WriteFeatureGuardsBegin(FILE* out, const char* file_name)
{
    assert(out != nullptr);
    assert(file_name != nullptr);

    for (const auto& guard : kFeatureGuards)
    {
        if (HasMatchingFeature(file_name, guard.match))
        {
            fwrite(guard.begin, sizeof(char), strlen(guard.begin), out);
        }
    }
}

static void WriteFeatureGuardsEnd(FILE* out, const char* file_name)
{
    assert(out != nullptr);
    assert(file_name != nullptr);

    for (const auto& guard : kFeatureGuards)
    {
        if (HasMatchingFeature(file_name, guard.match))
        {
            fwrite(guard.end, sizeof(char), strlen(guard.end), out);
        }
    }
}

static void MakeConstantName(const char* file_name, char* constant_name, size_t length)
{
    assert(file_name != nullptr);
    assert(constant_name != nullptr);
    assert(length > strlen(file_name));

    strncpy(constant_name, file_name, length);
    constant_name[length - 1] = '\0';
	
    auto* p = strchr(constant_name, '.');

    if (p != nullptr)
    {
        *p = '_';
    }
}

static void AppendFile(FILE* out, const char* file_name)
{
    assert(out != nullptr);
    assert(file_name != nullptr);

    auto* in = fopen(file_name, "r");
    assert(in != nullptr);

    char buffer[256];
    size_t read;

    while ((read = fread(buffer, sizeof(char), sizeof(buffer), in)) > 0)
    {
        fwrite(buffer, sizeof(char), read, out);
    }

    fclose(in);
}

static void BuildFinalContentHeader()
{
    auto* out = fopen("tmp.h", "w");
    assert(out != nullptr);

    fwrite("#ifndef CONTENT_H_\n", sizeof(char), 19, out);
    fwrite("#define CONTENT_H_\n\n", sizeof(char), 20, out);
    fwrite("#include <cstdint>\n", sizeof(char), 19, out);
    fwrite("#include \"httpd/httpd.h\"\n", sizeof(char), 25, out);

    AppendFile(out, "includes.h");
    AppendFile(out, "content.h");

    fwrite("\n\n#endif /* CONTENT_H_ */\n", sizeof(char), 26, out);

    fclose(out);

    const auto kI = remove("content.h");
    assert(kI == 0);

    const auto kJ = rename("tmp.h", "content.h");
    assert(kJ == 0);
}

static int ConvertToH(const char* file_name)
{
    assert(file_name != nullptr);

    printf("File to convert: %s, ", file_name);

    auto* file_in = fopen(file_name, "r");

    if (file_in == nullptr)
    {
        return 0;
    }

    const auto kFileNameLength = strlen(file_name);

    char file_name_out[128];
    assert((kFileNameLength + 3) < sizeof(file_name_out));

    const auto kHeaderNameLength = snprintf(file_name_out, sizeof(file_name_out), "%s.h", file_name);
    assert(kHeaderNameLength > 0);
    assert(kHeaderNameLength < static_cast<int>(sizeof(file_name_out)));

    printf("Header file: \"%s\", ", file_name_out);

    auto* file_out = fopen(file_name_out, "w");

    if (file_out == nullptr)
    {
        fclose(file_in);
        return 0;
    }

    WriteFeatureGuardsBegin(file_includes, file_name_out);

    char buffer[64];
    auto i = snprintf(buffer, sizeof(buffer), "#include \"%s\"\n", file_name_out);
    assert(i > 0);
    assert(i < static_cast<int>(sizeof(buffer)));

    fwrite(buffer, sizeof(char), i, file_includes);

    WriteFeatureGuardsEnd(file_includes, file_name_out);

    fwrite("static constexpr char ", sizeof(char), 22, file_out);

    char constant_name[128];
    assert((kFileNameLength + 1) < sizeof(constant_name));
    MakeConstantName(file_name, constant_name, kFileNameLength + 1);

    printf("Constant name: %s, ", constant_name);

    fwrite(constant_name, sizeof(char), strlen(constant_name), file_out);
    fwrite(constant_name, sizeof(char), strlen(constant_name), file_content);
    fwrite("[] = {\n", sizeof(char), 7, file_out);

    unsigned offset = 0;
    bool skip_leading_white_space = true;
    int file_size = 0;
    int c;

    while ((c = fgetc(file_in)) != EOF)
    {
        if (skip_leading_white_space)
        {
            if (c <= ' ')
            {
                continue;
            }

            skip_leading_white_space = false;
        }
        else if (c == '\n')
        {
            skip_leading_white_space = true;
        }

        i = snprintf(buffer, sizeof(buffer), "0x%02X,%c", c, (++offset % 16 == 0) ? '\n' : ' ');
        assert(i > 0);
        assert(i < static_cast<int>(sizeof(buffer)));

        fwrite(buffer, sizeof(char), i, file_out);
        file_size++;
    }

    fwrite("0x00\n};\n", sizeof(char), 8, file_out);

    fclose(file_in);
    fclose(file_out);

    printf("File size: %d\n", file_size);

    return file_size;
}

int main() // NOLINT
{
    file_includes = fopen("includes.h", "w");
    assert(file_includes != nullptr);

    file_content = fopen("content.h", "w");
    assert(file_content != nullptr);

    fwrite(kContentHeader, sizeof(char), strlen(kContentHeader), file_content);

    auto* dir = opendir(".");

    if (dir != nullptr)
    {
        struct dirent* dir_entry;

        while ((dir_entry = readdir(dir)) != nullptr)
        {
            if (dir_entry->d_name[0] == '.')
            {
                continue;
            }

            const auto kContentType = GetContentType(dir_entry->d_name);
            const auto kIsSupported = (kContentType != http::ContentTypes::kNotDefined);

            printf("%s -> %c\n", dir_entry->d_name, kIsSupported ? 'Y' : 'N');

            if (!kIsSupported)
            {
                continue;
            }

            WriteFeatureGuardsBegin(file_content, dir_entry->d_name);

            char buffer[256];
            auto i = snprintf(buffer, sizeof(buffer), "\t{ %u,\"%s\", ", 
				Fnv1a32Runtime(dir_entry->d_name, strlen(dir_entry->d_name)),
				dir_entry->d_name);
            assert(i > 0);
            assert(i < static_cast<int>(sizeof(buffer)));

            fwrite(buffer, sizeof(char), i, file_content);

            const auto kContentLength = ConvertToH(dir_entry->d_name);

            i = snprintf(buffer, sizeof(buffer), ", %d, static_cast<http::ContentTypes>(%d) },\n", kContentLength, static_cast<int>(kContentType));
            assert(i > 0);
            assert(i < static_cast<int>(sizeof(buffer)));

            fwrite(buffer, sizeof(char), i, file_content);

            WriteFeatureGuardsEnd(file_content, dir_entry->d_name);
        }

        closedir(dir);
    }

    fclose(file_includes);

    fwrite("};\n", sizeof(char), 3, file_content);
    fclose(file_content);

    BuildFinalContentHeader();

    return EXIT_SUCCESS;
}