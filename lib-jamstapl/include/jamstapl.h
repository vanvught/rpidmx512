/**
 * @file jamplayer.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef JAMSTAPL_H_
#define JAMSTAPL_H_

#include "jbiport.h"

extern "C" {
#include "../jbi_22/code/jbiexprt.h"
}

struct JamSTAPLDisplay {
	virtual ~JamSTAPLDisplay() {
	}

	virtual void JamShowInfo(const char *pInfo)=0;
	virtual void JamShowStatus(const char *pStatus, int nExitCode)=0;
};

class JamSTAPL {
public:
	JamSTAPL(PROGRAM_PTR pProgram, long nProgramSize, bool bVerbose = false) : m_pProgram(pProgram), m_nProgramSize(nProgramSize) {
		PlatformInit(bVerbose);
	}

	JBI_RETURN_TYPE CheckCRC(bool bVerbose = false);
	unsigned short GetCRC() const {
		return m_nCRC;
	}

	JBI_RETURN_TYPE PrintInfo();

	void ReadIdCode();
	void ReadUsercode();

	void CheckIdCode();

	void Erase();
	void Verify();
	void Program();

	JBI_RETURN_TYPE GetResult() const {
		return m_nExecResult;
	}

	const char *GetResultString() const;

	int GetExitCode() const {
		return m_nExitCode;
	}

	const char *GetExitCodeString() const;

	void SetMessage(const char *pMessage) {
		if (m_pJamSTAPLDisplay != nullptr) {
			m_pJamSTAPLDisplay->JamShowInfo(pMessage);
		}
	}

	void SetExportInteger(char *pKey, long nValue) {
		m_ExportInteger.pKey = pKey;
		m_ExportInteger.nValue = nValue;
	}

	const char *GetExportIntegerKey() const {
		return m_ExportInteger.pKey;
	}

	long GetExportIntegerInt() const {
		return m_ExportInteger.nValue;
	}

	void SetJamSTAPLDisplay(JamSTAPLDisplay *pJamSTAPLDisplay) {
		m_pJamSTAPLDisplay = pJamSTAPLDisplay;
	}

private:
	void PlatformInit(bool bVerbose);
	void Execute(const char *pAction);
	void DisplayStatus(const char *pAction);

private:
	PROGRAM_PTR m_pProgram;
	long m_nProgramSize;
	unsigned short m_nCRC{ 0 };
	int m_nResetJtag { 1 };
	long m_nErrorAddress;
	int m_nExitCode { -1 };
	int m_FormatVersion { 2 };
	JBI_RETURN_TYPE m_nExecResult { JBIC_ACTION_NOT_FOUND };

	struct ExportInteger {
		char *pKey;
		long nValue;
	};
	ExportInteger m_ExportInteger;

	JamSTAPLDisplay *m_pJamSTAPLDisplay{ nullptr };
};

#endif /* JAMSTAPL_H_ */
