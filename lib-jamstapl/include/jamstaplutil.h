/*
 * jamstaplutil.h
 *
 *  Created on: 11 jan. 2021
 *      Author: arjanvanvught
 */

#include <stdio.h>

class JamSTAPLUtil {
public:
	static long GetFileSize(const char *pFileName) {
		auto *pFile = fopen(pFileName, "r");

		if (pFile == nullptr) {
			return 0;
		}

		if (fseek(pFile, 0, SEEK_END) != 0) {
			return 0;
		}

		auto nFileSize = ftell(pFile);

		fclose(pFile);

		return nFileSize;
	}

	static int LoadFile(const char *pFileName, char *pBuffer, size_t nSize) {
		auto *pFile = fopen(pFileName, "r");

		if (pFile == nullptr) {
			return -1;
		}

		if (nSize <= 0) {
			return -3;
		}

		auto nBytes = fread(pBuffer, sizeof(char), nSize, pFile);

		if (nBytes != nSize) {
			return -2;
		}

		fclose(pFile);

		return 0;
	}
};
