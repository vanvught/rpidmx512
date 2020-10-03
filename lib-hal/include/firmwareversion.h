/**
 * @file firmwareversion.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef FIRMWAREVERSION_H_
#define FIRMWAREVERSION_H_

#define SOFTWARE_VERSION_LENGTH	3
#define GCC_DATE_LENGTH			11
#define GCC_TIME_LENGTH			8

struct TFirmwareVersion {
	char SoftwareVersion[SOFTWARE_VERSION_LENGTH];
	char BuildDate[GCC_DATE_LENGTH];
	char BuildTime[GCC_TIME_LENGTH];
};

class FirmwareVersion {
public:
	FirmwareVersion(const char *pVersion, const char *pDate, const char *pTime);
	~FirmwareVersion();

	void Print(const char *pTitle = nullptr);

	const struct TFirmwareVersion* GetVersion() {
		return &m_tFirmwareVersion;
	}

	const char* GetPrint() {
		return m_aPrint;
	}

	const char* GetSoftwareVersion() {
		return m_tFirmwareVersion.SoftwareVersion;
	}

	static FirmwareVersion* Get() {
		return s_pThis;
	}

private:
	struct TFirmwareVersion m_tFirmwareVersion;
	char m_aPrint[64];

	static FirmwareVersion *s_pThis;
};

#endif /* FIRMWAREVERSION_H_ */
