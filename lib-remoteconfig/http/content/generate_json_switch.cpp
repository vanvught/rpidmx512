/**
 * @file generate_json_switch.cpp
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

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <string.h>
#include <ctype.h>
#include <cassert>

static constexpr int MAX_LENGTH = 12;

static constexpr char JSON_ENDPOINTS[][MAX_LENGTH] = {
		"list",
		"version",
		"uptime",
		"display",
		"directory",
		"rdm",
		"queue",
		"tod",
		"phystatus",
		"portstatus",
		"vlantable",
		"status",
		"timedate",
		"rtcalarm",
		"polltable",
		"types"
};

inline uint16_t get_uint(const char *pString) {					/* djb2 */
	uint16_t hash = 5381;										/* djb2 */
	uint16_t c;													/* djb2 */
																/* djb2 */
	while ((c = *pString++) != '\0') {							/* djb2 */
		hash = static_cast<uint16_t>(((hash << 5) + hash) + c);	/* djb2 */
	}															/* djb2 */
																/* djb2 */
	return hash;												/* djb2 */
}																/* djb2 */

int main() {
	const auto pFile = fopen("json_switch.h", "w");
	assert(pFile != nullptr);


	fwrite("#ifndef JSON_SWITCH_H_\n", sizeof(char), 23, pFile);
	fwrite("#define JSON_SWITCH_H_\n\n", sizeof(char), 24, pFile);
	fwrite("#include <cstdint>\n\n", sizeof(char), 20, pFile);
	fwrite("namespace http {\n", sizeof(char), 17, pFile);
	fwrite("namespace json {\n", sizeof(char), 17, pFile);
	fwrite("namespace get {\n", sizeof(char), 16, pFile);

	char toUpper[MAX_LENGTH];

	for (uint32_t i = 0; i < sizeof(JSON_ENDPOINTS) / sizeof(JSON_ENDPOINTS[0]); i++) {
		size_t j;
		uint32_t k = 0;
		for (j = 0; j < strlen(JSON_ENDPOINTS[i]); j++) {
			if (isalpha(static_cast<int>(JSON_ENDPOINTS[i][j]))) {
				toUpper[k++] = toupper(static_cast<int>(JSON_ENDPOINTS[i][j]));
			}
		}
		toUpper[k] = '\0';

		char buffer[255];
		const auto nLength = snprintf(buffer, sizeof(buffer) - 1, "static constexpr uint16_t %-*s= 0x%.4x;\n", MAX_LENGTH, toUpper, get_uint(JSON_ENDPOINTS[i]));
		fwrite(buffer, sizeof(char), nLength, pFile);

		printf("%s", buffer);
	}

	fwrite("}\n}\n}\n\n", sizeof(char), 7, pFile);
	fclose(pFile);

	system("echo \'namespace http {\' >> json_switch.h");
	system("grep 'djb2' generate_json_switch.cpp | grep -v grep >> json_switch.h");
	system("echo \'}\n\' >> json_switch.h");
	system("echo \'#endif /* JSON_SWITCH_H_ */' >> json_switch.h");

	return EXIT_SUCCESS;
}
