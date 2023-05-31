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

	char buffer[32];
	auto i = snprintf(buffer, sizeof(buffer) - 1, "#include \"%s\"\n", pFileNameOut);
	assert(i < static_cast<int>(sizeof(buffer)));

	fwrite(buffer, sizeof(char), i, pFileIncludes);

	fwrite("static constexpr char ", sizeof(char), 22, pFileOut);

	char *pConstantName = new char[nFileNameLength + 1];
	assert(pConstantName != nullptr);

	strncpy(pConstantName, pFileName, nFileNameLength);

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
			if (c < ' ') {
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

				auto i = snprintf(pFileName, strlen(pDirEntry->d_name) + 8, "\t{ \"%s\", ", pDirEntry->d_name);
				fwrite(pFileName, sizeof(char), i, pFileContent);
				delete[] pFileName;

				auto nContentLength = convert_to_h(pDirEntry->d_name);

				char buffer[64];
				i = snprintf(buffer, sizeof(buffer) - 1, ", %d, static_cast<http::contentTypes>(%d)", nContentLength, static_cast<int>(contentType));
				assert(i < static_cast<int>(sizeof(buffer)));
				fwrite(buffer, sizeof(char), i, pFileContent);

				fwrite(" },\n", sizeof(char), 4, pFileContent);
			}
		}

		closedir(pDir);
	}

	fclose(pFileIncludes);

	fwrite("};\n", sizeof(char), 2, pFileContent);
	fclose(pFileContent);

	system("echo \'#include <cstdint>\n\' > tmp.h");
	system("echo \'#include \"httpd/httpd.h\"\n\' >> tmp.h");
	system("cat includes.h >> tmp.h");
	system("cat content.h >> tmp.h");
	system("rm content.h");
	system("mv tmp.h content.h");

	return EXIT_SUCCESS;
}
