/**
 * @file generate_content.cpp
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org */


#undef NDEBUG

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <cassert>

#include "http/http.h"

static constexpr char kSupportedExtensions[static_cast<int>(http::contentTypes::NOT_DEFINED)][8] = 
{
	"html", 
	"css", 
	"js", 
	"json"
};

static constexpr char kContentHeader[] =
    "\n"
    "struct FilesContent {\n"
    "\tconst char *pFileName;\n"
    "\tconst char *pContent;\n"
    "\tconst uint32_t nContentLength;\n"
    "\tconst http::contentTypes contentType;\n"
    "};\n\n"
    "static constexpr struct FilesContent HttpContent[] = {\n";

static constexpr char kHaveDmxBegin[] = "#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))\n";
static constexpr char kHaveDmxEnd[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */\n";

static constexpr char kHaveRdmBegin[] = "#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)\n";
static constexpr char kHaveRdmEnd[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */\n";

static constexpr char kHaveShowfileBegin[] = "#if defined (NODE_SHOWFILE)\n";
static constexpr char kHaveShowfileEnd[] = "#endif /* (NODE_SHOWFILE) */\n";

static constexpr char kHaveTimeBegin[] = "#if !defined (CONFIG_HTTP_HTML_NO_TIME)\n";
static constexpr char kHaveTimeEnd[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */\n";

static constexpr char kHaveRtcBegin[] = "#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)\n";
static constexpr char kHaveRtcEnd[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */\n";

static FILE* file_content;
static FILE* file_includes;

static http::contentTypes GetContentType(const char* file_name)
{
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
                    return static_cast<http::contentTypes>(i);
                }
            }
        }
    }

    return http::contentTypes::NOT_DEFINED;
}

static int ConvertToH(const char* file_name)
{
    printf("File to convert: %s, ", file_name);

    auto* pFileIn = fopen(file_name, "r");

    if (pFileIn == nullptr)
    {
        return 0;
    }

    const auto nFileNameLength = strlen(file_name);

    auto* pFileNameOut = new char[nFileNameLength + 3];
    assert(pFileNameOut != nullptr);

    snprintf(pFileNameOut, nFileNameLength + 3, "%s.h", file_name);

    printf("Header file: \"%s\", ", pFileNameOut);

    auto* pFileOut = fopen(pFileNameOut, "w");

    if (pFileOut == nullptr)
    {
        delete[] pFileNameOut;
        fclose(pFileIn);
        return 0;
    }

    char buffer[64];

    const auto bHasDMX = (strstr(pFileNameOut, "dmx") != nullptr);

    if (bHasDMX)
    {
        fwrite(kHaveDmxBegin, sizeof(char), sizeof(kHaveDmxBegin) - 1, file_includes);
    }

    const auto bHasRDM = (strstr(pFileNameOut, "rdm") != nullptr);

    if (bHasRDM)
    {
        fwrite(kHaveRdmBegin, sizeof(char), sizeof(kHaveRdmBegin) - 1, file_includes);
    }

    const auto bHasSHOWFILE = (strstr(pFileNameOut, "showfile") != nullptr);

    if (bHasSHOWFILE)
    {
        fwrite(kHaveShowfileBegin, sizeof(char), sizeof(kHaveShowfileBegin) - 1, file_includes);
    }

    const auto bHasTIME = (strstr(pFileNameOut, "time") != nullptr);

    if (bHasTIME)
    {
        fwrite(kHaveTimeBegin, sizeof(char), sizeof(kHaveTimeBegin) - 1, file_includes);
    }

    const auto bHasRTC = (strstr(pFileNameOut, "rtc") != nullptr);

    if (bHasRTC)
    {
        fwrite(kHaveRtcBegin, sizeof(char), sizeof(kHaveRtcBegin) - 1, file_includes);
    }

    auto i = snprintf(buffer, sizeof(buffer) - 1, "#%sinclude \"%s\"\n", (bHasDMX || bHasRDM || bHasSHOWFILE || bHasTIME || bHasRTC) ? " " : "", pFileNameOut);
    assert(i < static_cast<int>(sizeof(buffer)));

    fwrite(buffer, sizeof(char), i, file_includes);

    if (bHasDMX)
    {
        fwrite(kHaveDmxEnd, sizeof(char), sizeof(kHaveDmxEnd) - 1, file_includes);
    }

    if (bHasRDM)
    {
        fwrite(kHaveRdmEnd, sizeof(char), sizeof(kHaveRdmEnd) - 1, file_includes);
    }

    if (bHasSHOWFILE)
    {
        fwrite(kHaveShowfileEnd, sizeof(char), sizeof(kHaveShowfileEnd) - 1, file_includes);
    }

    if (bHasTIME)
    {
        fwrite(kHaveTimeEnd, sizeof(char), sizeof(kHaveTimeEnd) - 1, file_includes);
    }

    if (bHasRTC)
    {
        fwrite(kHaveRtcEnd, sizeof(char), sizeof(kHaveRtcEnd) - 1, file_includes);
    }

    fwrite("static constexpr char ", sizeof(char), 22, pFileOut);

    char* pConstantName = new char[nFileNameLength + 1];
    assert(pConstantName != nullptr);

    strncpy(pConstantName, file_name, nFileNameLength);
    pConstantName[nFileNameLength] = '\0';

    auto* p = strstr(pConstantName, ".");

    if (p != nullptr)
    {
        *p = '_';
    }

    printf("Constant name: %s, ", pConstantName);

    fwrite(pConstantName, sizeof(char), strlen(pConstantName), pFileOut);
    fwrite(pConstantName, sizeof(char), strlen(pConstantName), file_content);
    fwrite("[] = {\n", sizeof(char), 7, pFileOut);

    unsigned nOffset = 0;
    auto doRemoveWhiteSpaces = true;
    int nFileSize = 0;
    int c;

    while ((c = fgetc(pFileIn)) != EOF)
    {
        if (doRemoveWhiteSpaces)
        {
            if (c <= ' ')
            {
                continue;
            }
            else
            {
                doRemoveWhiteSpaces = false;
            }
        }
        else
        {
            if (c == '\n')
            {
                doRemoveWhiteSpaces = true;
            }
        }

        i = snprintf(buffer, sizeof(buffer) - 1, "0x%02X,%c", c, ++nOffset % 16 == 0 ? '\n' : ' ');
        assert(i < static_cast<int>(sizeof(buffer)));

        fwrite(buffer, sizeof(char), i, pFileOut);

        nFileSize++;
    }

    fwrite("0x00\n};\n", sizeof(char), 8, pFileOut);

    delete[] pFileNameOut;
    delete[] pConstantName;

    fclose(pFileIn);
    fclose(pFileOut);

    printf("File size: %d\n", nFileSize);

    return nFileSize;
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
            const auto kIsSupported = (kContentType != http::contentTypes::NOT_DEFINED);
           
			 printf("%s -> %c\n", dir_entry->d_name, kIsSupported ? 'Y' : 'N');

            if (kIsSupported)
            {
                auto* pFileName = new char[strlen(dir_entry->d_name) + 9];
                assert(pFileName != nullptr);

                const auto bHasDMX = (strstr(dir_entry->d_name, "dmx") != nullptr);
                const auto bHasRDM = (strstr(dir_entry->d_name, "rdm") != nullptr);
                const auto bHasSHOWFILE = (strstr(dir_entry->d_name, "showfile") != nullptr);
                const auto bHasTIME = (strstr(dir_entry->d_name, "time") != nullptr);
                const auto bHasRTC = (strstr(dir_entry->d_name, "rtc") != nullptr);

                if (bHasDMX)
                {
                    fwrite(kHaveDmxBegin, sizeof(char), sizeof(kHaveDmxBegin) - 1, file_content);
                }

                if (bHasRDM)
                {
                    fwrite(kHaveRdmBegin, sizeof(char), sizeof(kHaveRdmBegin) - 1, file_content);
                }

                if (bHasSHOWFILE)
                {
                    fwrite(kHaveShowfileBegin, sizeof(char), sizeof(kHaveShowfileBegin) - 1, file_content);
                }

                if (bHasTIME)
                {
                    fwrite(kHaveTimeBegin, sizeof(char), sizeof(kHaveTimeBegin) - 1, file_content);
                }

                if (bHasRTC)
                {
                    fwrite(kHaveRtcBegin, sizeof(char), sizeof(kHaveRtcBegin) - 1, file_content);
                }

                auto i = snprintf(pFileName, strlen(dir_entry->d_name) + 8, "\t{ \"%s\", ", dir_entry->d_name);
                fwrite(pFileName, sizeof(char), i, file_content);
                delete[] pFileName;

                const auto kContentLength = ConvertToH(dir_entry->d_name);

                char buffer[64];
                i = snprintf(buffer, sizeof(buffer) - 1, ", %d, static_cast<http::contentTypes>(%d)", kContentLength, static_cast<int>(kContentType));
                assert(i < static_cast<int>(sizeof(buffer)));
                fwrite(buffer, sizeof(char), i, file_content);

                fwrite(" },\n", sizeof(char), 4, file_content);

                if (bHasDMX)
                {
                    fwrite(kHaveDmxEnd, sizeof(char), sizeof(kHaveDmxEnd) - 1, file_content);
                }

                if (bHasRDM)
                {
                    fwrite(kHaveRdmEnd, sizeof(char), sizeof(kHaveRdmEnd) - 1, file_content);
                }

                if (bHasSHOWFILE)
                {
                    fwrite(kHaveShowfileEnd, sizeof(char), sizeof(kHaveShowfileEnd) - 1, file_content);
                }

                if (bHasTIME)
                {
                    fwrite(kHaveTimeEnd, sizeof(char), sizeof(kHaveTimeEnd) - 1, file_content);
                }

                if (bHasRTC)
                {
                    fwrite(kHaveRtcEnd, sizeof(char), sizeof(kHaveRtcEnd) - 1, file_content);
                }
            }
        }

        closedir(dir);
    }

    fclose(file_includes);

    fwrite("};\n", sizeof(char), 2, file_content);
    fclose(file_content);

    system("echo \'#ifndef CONTENT_H_' > tmp.h");
    system("echo \'#define CONTENT_H_\n' >> tmp.h");
    system("echo \'#include <cstdint>\n\' >> tmp.h");
    system("echo \'#include \"httpd/httpd.h\"\n\' >> tmp.h");
    system("cat includes.h >> tmp.h");
    system("cat content.h >> tmp.h");
    system("echo \'\n\n#endif /* CONTENT_H_ */' >> tmp.h");
    system("rm content.h");
    system("mv tmp.h content.h");

    return EXIT_SUCCESS;
}
