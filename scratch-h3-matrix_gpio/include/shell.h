/**
 * @file shell.h
 *
 */
/* Copyright (C) 2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef H3_SHELL_H_
#define H3_SHELL_H_

#include <stdint.h>

// Firmware specific BEGIN
#if defined (LTC_READER)
# include "ltc.h"
#endif
// Firmware specific END

namespace shell {
enum class CmdIndex: uint32_t {
	REBOOT,
	INFO,
	SET,
	GET,
	DHCP,
	DATE,
	HWCLOCK,
#ifndef NDEBUG
	I2CDETECT,
	DUMP,
	MEM,
	NTP,
	PTP,
	GPS,
#endif
	HELP
};
static constexpr auto BUFLEN = 196;
static constexpr auto MAXARG = 4;
}  // namespace shell

class Shell {
public:
	Shell();

	void Run();

#if defined (LTC_READER)
	void SetSource(ltc::source ltcSource) {
		m_ltcSource = ltcSource;
	}
#endif

private:
	// shell.cpp
	const char *ReadLine(uint32_t &nLength);
	uint32_t ValidateCmd(uint32_t nLength, shell::CmdIndex &nCmdIndex);
	void ValidateArg(uint32_t nOffset, uint32_t nLength);
	// shellcmd.cpp
	uint32_t hexadecimalToDecimal(const char *pHexValue, uint32_t nLength);
	void CmdReboot();
	void CmdInfo();
	void CmdSet();
	void CmdGet();
	void CmdDhcp();
	void CmdDate();
	void CmdHwClock();
#ifndef NDEBUG
	void CmdI2cDetect();
	void CmdDump();
	void CmdMem();
	void CmdNtp();
	void CmdPtp();
	void CmdGps();
#endif
	void CmdHelp();

private:
	bool m_bIsEndOfLine{false};		
	uint32_t m_nLength{0};
	char m_Buffer[shell::BUFLEN];
	uint32_t m_Argc{0};
	char *m_Argv[shell::MAXARG]{nullptr};
	uint32_t m_nArgvLength[shell::MAXARG];
	bool m_bShownPrompt{false};

// Firmware specific BEGIN
#if defined (LTC_READER)
	ltc::source m_ltcSource{ltc::UNDEFINED};
#endif
// Firmware specific END
};

#endif /* H3_SHELL_H_ */
