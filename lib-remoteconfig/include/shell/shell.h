/**
 * @file shell.h
 *
 */
/* Copyright (C) 2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SHELL_H_
#define SHELL_H_

#include <cstdint>
#include <cstdarg>

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
	PHY,
#if !defined (DISABLE_RTC)
	HWCLOCK,
#endif
#if defined (DEBUG_I2C)
	I2CDETECT,
#endif
	DUMP,
	MEM,
#if defined (ENABLE_NTP_CLIENT)
	NTP,
#endif
#if defined (CONFIG_SHELL_GPS)
	GPS,
#endif
#if (PHY_TYPE == RTL8201F)
	PHY_TYPE_RTL8201F,
#endif
	HELP
};
static constexpr auto BUFLEN = 196;
static constexpr auto MAXARG = 4;

namespace msg {
namespace error {
static constexpr char INVALID[] = "Invalid command.\n";
static constexpr char INTERNAL[] = "Internal error.\n";
}  // namespace error
}  // namespace msg
}  // namespace shell

class Shell {
public:
	Shell();

	void Run();

#if defined (LTC_READER)
	void SetSource(ltc::Source ltcSource) {
		m_ltcSource = ltcSource;
	}
#endif

private:
	void Puts(const char *pString);
	int Printf(const char* fmt, ...);
	// shell.cpp
	const char *ReadLine(uint32_t& nLength);
	uint32_t ValidateCmd(const uint32_t nLength, shell::CmdIndex &nCmdIndex);
	void ValidateArg(uint32_t nOffset, const uint32_t nLength);
	// shellcmd.cpp
	void CmdReboot();
	void CmdInfo();
	void CmdSet();
	void CmdGet();
	void CmdDhcp();
	void CmdDate();
	void CmdPhy();
	void CmdHwClock();
	void CmdI2cDetect();
	void CmdDump();
	void CmdMem();
	void CmdNtp();
	void CmdGps();
	void CmdPhyTypeRTL8201F();
	void CmdHelp();

private:
	char m_Buffer[shell::BUFLEN];
	uint32_t m_Argc { 0 };
	uint32_t m_nLength { 0 };
	char *m_Argv[shell::MAXARG] { nullptr };
	uint16_t m_nArgvLength[shell::MAXARG];
	bool m_bShownPrompt { false };
	bool m_bIsEndOfLine { false };

// Firmware specific BEGIN
#if defined (LTC_READER)
	ltc::Source m_ltcSource{ltc::Source::UNDEFINED};
#endif
// Firmware specific END
};

#endif /* SHELL_H_ */
