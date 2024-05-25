/**
 * @file generate_content.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <cassert>

#include "httpd/httpd.h"

static constexpr char supported_extensions[static_cast<int>(http::contentTypes::NOT_DEFINED)][8] = {
		"html",
		"css",
		"js",
		"json"
};

static constexpr char content_header[] =
		"\n"
		"struct FilesContent {\n"
		"\tconst char *pFileName;\n"
		"\tconst char *pContent;\n"
		"\tconst uint32_t nContentLength;\n"
		"\tconst http::contentTypes contentType;\n"
		"};\n\n"
		"static constexpr struct FilesContent HttpContent[] = {\n";

static constexpr char HAVE_DSA_BEGIN[] = "#if defined (ENABLE_PHY_SWITCH)\n";
static constexpr char HAVE_DSA_END[] = "#endif /* (ENABLE_PHY_SWITCH) */\n";

static constexpr char HAVE_DMX_BEGIN[] = "#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))\n";
static constexpr char HAVE_DMX_END[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */\n";

static constexpr char HAVE_RDM_BEGIN[] = "#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)\n";
static constexpr char HAVE_RDM_END[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */\n";

static constexpr char HAVE_SHOWFILE_BEGIN[] = "#if defined (NODE_SHOWFILE)\n";
static constexpr char HAVE_SHOWFILE_END[] = "#endif /* (NODE_SHOWFILE) */\n";

static constexpr char HAVE_TIME_BEGIN[] = "#if !defined (CONFIG_HTTP_HTML_NO_TIME)\n";
static constexpr char HAVE_TIME_END[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */\n";

static constexpr char HAVE_RTC_BEGIN[] = "#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)\n";
static constexpr char HAVE_RTC_END[] = "#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */\n";

static FILE *pFileContent;
static FILE *pFileIncludes;

static http::contentTypes getContentType(const char *pFileName) {
	for (int i = 0; i < static_cast<int>(http::contentTypes::NOT_DEFINED); i++) {
		const auto l = strlen(pFileName);
		const auto e = strlen(supported_extensions[i]);

		if (l > (e + 2)) {
			if (pFileName[l - e - 1] == '.') {
				if (strcmp(&pFileName[l - e], supported_extensions[i]) == 0) {
					return static_cast<http::contentTypes>(i);
				}
			}
		}
	}

	return http::contentTypes::NOT_DEFINED;
}

static int convert_to_h(const char *pFileName) {
	printf("File to convert: %s, ", pFileName);

	auto *pFileIn = fopen(pFileName, "r");

	if (pFileIn == nullptr) {
		return 0;
	}

	const auto nFileNameLength = strlen(pFileName);

	auto *pFileNameOut = new char[nFileNameLength + 3];
	assert(pFileNameOut != nullptr);

	snprintf(pFileNameOut, nFileNameLength + 3, "%s.h", pFileName);

	printf("Header file: \"%s\", ", pFileNameOut);

	auto *pFileOut = fopen(pFileNameOut, "w");

	if (pFileOut == nullptr) {
		delete[] pFileNameOut;
		fclose(pFileIn);
		return 0;
	}

	char buffer[64];

	const auto bHasDSA = (strstr(pFileNameOut, "dsa") != nullptr);

	if (bHasDSA)  {
		fwrite(HAVE_DSA_BEGIN, sizeof(char),sizeof(HAVE_DSA_BEGIN) - 1, pFileIncludes);
	}

	const auto bHasDMX = (strstr(pFileNameOut, "dmx") != nullptr);

	if (bHasDMX)  {
		fwrite(HAVE_DMX_BEGIN, sizeof(char),sizeof(HAVE_DMX_BEGIN) - 1, pFileIncludes);
	}

	const auto bHasRDM = (strstr(pFileNameOut, "rdm") != nullptr);

	if (bHasRDM)  {
		fwrite(HAVE_RDM_BEGIN, sizeof(char),sizeof(HAVE_RDM_BEGIN) - 1, pFileIncludes);
	}

	const auto bHasSHOWFILE = (strstr(pFileNameOut, "showfile") != nullptr);

	if (bHasSHOWFILE)  {
		fwrite(HAVE_SHOWFILE_BEGIN, sizeof(char),sizeof(HAVE_SHOWFILE_BEGIN) - 1, pFileIncludes);
	}

	const auto bHasTIME = (strstr(pFileNameOut, "time") != nullptr);

	if (bHasTIME)  {
		fwrite(HAVE_TIME_BEGIN, sizeof(char),sizeof(HAVE_TIME_BEGIN) - 1, pFileIncludes);
	}

	const auto bHasRTC = (strstr(pFileNameOut, "rtc") != nullptr);

	if (bHasRTC)  {
		fwrite(HAVE_RTC_BEGIN, sizeof(char),sizeof(HAVE_RTC_BEGIN) - 1, pFileIncludes);
	}

	auto i = snprintf(buffer, sizeof(buffer) - 1, "#%sinclude \"%s\"\n", (bHasDSA || bHasDMX || bHasRDM || bHasSHOWFILE || bHasTIME || bHasRTC) ? " " : "" , pFileNameOut);
	assert(i < static_cast<int>(sizeof(buffer)));

	fwrite(buffer, sizeof(char), i, pFileIncludes);

	if (bHasDSA)  {
		fwrite(HAVE_DSA_END, sizeof(char),sizeof(HAVE_DSA_END) - 1, pFileIncludes);
	}

	if (bHasDMX)  {
		fwrite(HAVE_DMX_END, sizeof(char),sizeof(HAVE_DMX_END) - 1, pFileIncludes);
	}

	if (bHasRDM)  {
		fwrite(HAVE_RDM_END, sizeof(char),sizeof(HAVE_RDM_END) - 1, pFileIncludes);
	}

	if (bHasSHOWFILE)  {
		fwrite(HAVE_SHOWFILE_END, sizeof(char),sizeof(HAVE_SHOWFILE_END) - 1, pFileIncludes);
	}

	if (bHasTIME)  {
		fwrite(HAVE_TIME_END, sizeof(char),sizeof(HAVE_TIME_END) - 1, pFileIncludes);
	}

	if (bHasRTC)  {
		fwrite(HAVE_RTC_END, sizeof(char),sizeof(HAVE_RTC_END) - 1, pFileIncludes);
	}

	fwrite("static constexpr char ", sizeof(char), 22, pFileOut);

	char *pConstantName = new char[nFileNameLength + 1];
	assert(pConstantName != nullptr);

	strncpy(pConstantName, pFileName, nFileNameLength);
	pConstantName[nFileNameLength] = '\0';

	auto *p = strstr(pConstantName, ".");

	if (p != nullptr) {
		*p = '_';
	}

	printf("Constant name: %s, ", pConstantName);

	fwrite(pConstantName, sizeof(char), strlen(pConstantName), pFileOut);
	fwrite(pConstantName, sizeof(char), strlen(pConstantName), pFileContent);
	fwrite("[] = {\n", sizeof(char), 7, pFileOut);

	unsigned nOffset = 0;
	auto doRemoveWhiteSpaces = true;
	int nFileSize = 0;
	int c;

	while ((c = fgetc (pFileIn)) != EOF) {
		if (doRemoveWhiteSpaces) {
			if (c <= ' ') {
				continue;
			} else {
				doRemoveWhiteSpaces = false;
			}
		} else {
			if (c == '\n') {
				doRemoveWhiteSpaces = true;
			}
		}

		i = snprintf(buffer, sizeof(buffer) - 1, "0x%02X,%c", c, ++nOffset % 16 == 0 ? '\n' : ' ');
		assert(i < static_cast<int>(sizeof(buffer)));

		fwrite(buffer, sizeof(char), i, pFileOut);

		nFileSize++;
	}

	fwrite("0x00\n};\n", sizeof(char), 8, pFileOut);

	delete [] pFileNameOut;
	delete [] pConstantName;

	fclose(pFileIn);
	fclose(pFileOut);

	printf("File size: %d\n", nFileSize);

	return nFileSize;
}

int main() {
	pFileIncludes = fopen("includes.h", "w");
	assert(pFileIncludes != nullptr);

	pFileContent = fopen("content.h", "w");
	assert(pFileContent != nullptr);

	fwrite(content_header, sizeof(char), strlen(content_header), pFileContent);

	auto *pDir = opendir(".");

	if (pDir != nullptr) {
		struct dirent *pDirEntry;
		while ((pDirEntry = readdir(pDir)) != nullptr) {
			if (pDirEntry->d_name[0] == '.') {
				continue;
			}

			auto contentType = getContentType(pDirEntry->d_name);
			const auto isSupported = (contentType != http::contentTypes::NOT_DEFINED);
			printf("%s -> %c\n", pDirEntry->d_name, isSupported ? 'Y' : 'N');

			if (isSupported) {
				auto *pFileName = new char[strlen(pDirEntry->d_name) + 9];
				assert(pFileName != nullptr);

				const auto bHasDSA = (strstr(pDirEntry->d_name, "dsa") != nullptr);
				const auto bHasDMX = (strstr(pDirEntry->d_name, "dmx") != nullptr);
				const auto bHasRDM = (strstr(pDirEntry->d_name, "rdm") != nullptr);
				const auto bHasSHOWFILE = (strstr(pDirEntry->d_name, "showfile") != nullptr);
				const auto bHasTIME = (strstr(pDirEntry->d_name, "time") != nullptr);
				const auto bHasRTC = (strstr(pDirEntry->d_name, "rtc") != nullptr);

				if (bHasDSA)  {
					fwrite(HAVE_DSA_BEGIN, sizeof(char),sizeof(HAVE_DSA_BEGIN) - 1, pFileContent);
				}

				if (bHasDMX)  {
					fwrite(HAVE_DMX_BEGIN, sizeof(char),sizeof(HAVE_DMX_BEGIN) - 1, pFileContent);
				}

				if (bHasRDM)  {
					fwrite(HAVE_RDM_BEGIN, sizeof(char),sizeof(HAVE_RDM_BEGIN) - 1, pFileContent);
				}

				if (bHasSHOWFILE)  {
					fwrite(HAVE_SHOWFILE_BEGIN, sizeof(char),sizeof(HAVE_SHOWFILE_BEGIN) - 1, pFileContent);
				}

				if (bHasTIME)  {
					fwrite(HAVE_TIME_BEGIN, sizeof(char),sizeof(HAVE_TIME_BEGIN) - 1, pFileContent);
				}

				if (bHasRTC)  {
					fwrite(HAVE_RTC_BEGIN, sizeof(char),sizeof(HAVE_RTC_BEGIN) - 1, pFileContent);
				}

				auto i = snprintf(pFileName, strlen(pDirEntry->d_name) + 8, "\t{ \"%s\", ", pDirEntry->d_name);
				fwrite(pFileName, sizeof(char), i, pFileContent);
				delete[] pFileName;

				auto nContentLength = convert_to_h(pDirEntry->d_name);

				char buffer[64];
				i = snprintf(buffer, sizeof(buffer) - 1, ", %d, static_cast<http::contentTypes>(%d)", nContentLength, static_cast<int>(contentType));
				assert(i < static_cast<int>(sizeof(buffer)));
				fwrite(buffer, sizeof(char), i, pFileContent);

				fwrite(" },\n", sizeof(char), 4, pFileContent);

				if (bHasDSA)  {
					fwrite(HAVE_DSA_END, sizeof(char),sizeof(HAVE_DSA_END) - 1, pFileContent);
				}

				if (bHasDMX)  {
					fwrite(HAVE_DMX_END, sizeof(char),sizeof(HAVE_DMX_END) - 1, pFileContent);
				}

				if (bHasRDM)  {
					fwrite(HAVE_RDM_END, sizeof(char),sizeof(HAVE_RDM_END) - 1, pFileContent);
				}

				if (bHasSHOWFILE)  {
					fwrite(HAVE_SHOWFILE_END, sizeof(char),sizeof(HAVE_SHOWFILE_END) - 1, pFileContent);
				}

				if (bHasTIME)  {
					fwrite(HAVE_TIME_END, sizeof(char),sizeof(HAVE_TIME_END) - 1, pFileContent);
				}

				if (bHasRTC)  {
					fwrite(HAVE_RTC_END, sizeof(char),sizeof(HAVE_RTC_END) - 1, pFileContent);
				}
			}
		}

		closedir(pDir);
	}

	fclose(pFileIncludes);

	fwrite("};\n", sizeof(char), 2, pFileContent);
	fclose(pFileContent);

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
