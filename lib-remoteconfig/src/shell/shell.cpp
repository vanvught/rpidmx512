/**
 * @file shell.cpp
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

#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstdarg>

#include "shell/shell.h"

#if defined (GD32) // PHY_TYPE is defined here
# include "gd32.h"
#endif

#include "debug.h"

struct ShellCommands {
	const char *pName;
	const uint32_t nArgc;
};

static constexpr ShellCommands cmd_table[] = {
		{ "reboot", 0 },
		{ "info", 0 },
		{ "set", 2 },
		{ "get", 2 },
		{ "dhcp", 0 },
		{ "date", 0 },
		{ "phy", 0 },
#if !defined (DISABLE_RTC)
		{ "hwclock", 1 },
#endif
#if defined (DEBUG_I2C)
		{ "i2cdetect", 0 },
#endif
		{ "dump", 1 },
		{ "mem", 2 },
#if defined (ENABLE_NTP_CLIENT)
		{ "ntp", 1 },
#endif
#if defined (CONFIG_SHELL_GPS)
		{ "gps", 1 },
#endif
#if (PHY_TYPE == RTL8201F)
		{ "rtl8201f" , 2},
#endif
		{ "?", 0 } };

namespace shell {
static constexpr auto TABLE_SIZE = sizeof(cmd_table) / sizeof(cmd_table[0]);
namespace msg {
static constexpr char CMD_PROMPT[] = "> ";
static constexpr char CMD_NOT_FOUND[] = "Command not found: ";
static constexpr char CMD_WRONG_ARGUMENTS[] = "Wrong arguments\n";
}  // namespace msg
}  // namespace shell

Shell::Shell() {
	DEBUG_PRINTF("TABLE_SIZE=%d", shell::TABLE_SIZE);
}

int Shell::Printf(const char* fmt, ...) {
	char s[128];

	va_list arp;

	va_start(arp, fmt);

	int i = vsnprintf(s, sizeof(s) -1, fmt, arp);
	va_end(arp);

	Puts(s);

	return i;
}

uint32_t Shell::ValidateCmd(const uint32_t nLength,  shell::CmdIndex &nCmdIndex) {
	uint32_t i;

	m_Argc = 0;

	for (i = 0; i < nLength; i++) {
		if ((m_Buffer[i] == ' ') || (m_Buffer[i] == '\t')) {
			m_Buffer[i] = '\0';
			break;
		}
	}

	for (uint32_t j = 0; j <  shell::TABLE_SIZE; j++) {
		if (0 == strcmp(m_Buffer, cmd_table[j].pName)) {
			nCmdIndex = static_cast< shell::CmdIndex>(j);
			return i + 1;
		}
	}

	return 0;
}

void Shell::ValidateArg(uint32_t nOffset, const uint32_t nLength) {
	if (nOffset > nLength) {
		return;
	}

	while (nOffset < nLength && ((m_Buffer[nOffset] == ' ') || (m_Buffer[nOffset] == '\t'))) {
		nOffset++;
	}

	if (nOffset == nLength) {
		return;
	}

	auto nArgvStart = nOffset;
	m_Argv[0] = &m_Buffer[nOffset++];
	m_Argc = 1;

	uint32_t i, j = 1;

	for (i = nOffset; i < nLength; i++) {
		if ((m_Buffer[i] > ' ') && (m_Buffer[i] < 127)) {
			continue;
		}

		if ((m_Buffer[i] == ' ') || (m_Buffer[i] == '\t')) {
			if (j <  shell::MAXARG) {
				m_nArgvLength[j - 1] = static_cast<uint16_t>(i - nArgvStart);
			}
			while (i < nLength && ((m_Buffer[i] == ' ') || (m_Buffer[i] == '\t'))) {
				m_Buffer[i++] = '\0';
			}
			if (j <  shell::MAXARG) {
				nArgvStart = i;
				m_Argv[j++] = &m_Buffer[i];
			}
			m_Argc++;
		}
	}

	if (j <  shell::MAXARG) {
		m_nArgvLength[j - 1] = static_cast<uint16_t>(i - nArgvStart);
	}

#ifndef NDEBUG
	DEBUG_PRINTF("m_Argc=%d", m_Argc);
	for (uint32_t i = 0; i < m_Argc; i++) {
		Printf("%d:[%s]{%d}\n", i, m_Argv[i], m_nArgvLength[i]);
	}
#endif
}

void Shell::CmdHelp() {
	Puts("http://www.orangepi-dmx.org/orange-pi-dmx512-rdm/uart0-shell\n");

	uint32_t i  = 0;
	for (auto& cmd : cmd_table) {
		Printf("%2u: %s <%u>\n", i++, cmd.pName, cmd.nArgc);
	}

	Puts("");
}

void Shell::Run() {	
	if (__builtin_expect((!m_bShownPrompt), 1)) {
		Puts(shell::msg::CMD_PROMPT);
		m_bShownPrompt = true;
	}
	
	uint32_t nLength;

	if (__builtin_expect((ReadLine(nLength) == nullptr), 1)) {
		return;
	}
	
	Puts("\n");

	m_bShownPrompt = false; // next time round, we show the prompt.

	uint32_t nOffset;
	shell::CmdIndex nCmdIndex;

	if ((nOffset = ValidateCmd(nLength, nCmdIndex)) == 0) {
		Printf("%s %s\n", shell::msg::CMD_NOT_FOUND, m_Buffer);
		return;
	}

	ValidateArg(nOffset, nLength);

	if (m_Argc != cmd_table[static_cast<uint32_t>(nCmdIndex)].nArgc) {
		Puts( shell::msg::CMD_WRONG_ARGUMENTS);
		return;
	}

	switch (nCmdIndex) {
	case shell::CmdIndex::REBOOT:
		CmdReboot();
		break;
	case shell::CmdIndex::INFO:
		CmdInfo();
		break;
	case shell::CmdIndex::SET:
		CmdSet();
		break;
	case shell::CmdIndex::GET:
		CmdGet();
		break;
	case shell::CmdIndex::DHCP:
		CmdDhcp();
		break;
	case shell::CmdIndex::DATE:
		CmdDate();
		break;
	case shell::CmdIndex::PHY:
		CmdPhy();
		break;
#if !defined(DISABLE_RTC)
	case shell::CmdIndex::HWCLOCK:
		CmdHwClock();
		break;
#endif
#if defined (DEBUG_I2C)
	case shell::CmdIndex::I2CDETECT:
		CmdI2cDetect();
		break;
#endif
	case shell::CmdIndex::DUMP:
		CmdDump();
		break;
	case shell::CmdIndex::MEM:
		CmdMem();
		break;
#if defined (ENABLE_NTP_CLIENT)
	case shell::CmdIndex::NTP:
		CmdNtp();
		break;
#endif
#if defined (CONFIG_SHELL_GPS)
	case shell::CmdIndex::GPS:
		CmdGps();
		break;
#endif
#if (PHY_TYPE == RTL8201F)
	case  shell::CmdIndex::PHY_TYPE_RTL8201F:
		CmdPhyTypeRTL8201F();
		break;
#endif
	case shell::CmdIndex::HELP:
		CmdHelp();
		break;
	default:
		break;
	}
}
