/**
 * @file shell.cpp
 *
 */
/* Copyright (C) 2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2020-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "debug.h"

struct TCommands {
	const char *pName;
	const uint32_t nArgc;
};

static constexpr TCommands cmd_table[] = {
		{ "reboot", 0 },
		{ "info", 0 },
		{ "set", 2},
		{ "get", 2},
		{ "dhcp", 0},
		{ "date", 0},
		{ "hwclock", 1},
#ifndef NDEBUG
		{ "i2cdetect" , 0},
		{ "dump" , 1},
		{ "mem" , 2},
		{ "ntp" , 1},
		{ "gps" , 1},
#endif
		{ "?", 0 }
};

namespace shell {
static constexpr auto TABLE_SIZE = sizeof(cmd_table) / sizeof(cmd_table[0]);
namespace msg {
static constexpr char CMD_PROMPT[] = "> ";
static constexpr char CMD_NOT_FOUND[] = "Command not found: ";
static constexpr char CMD_WRONG_ARGUMENTS[] = "Wrong arguments\n";
}  // namespace msg
}  // namespace shell

using namespace shell;

Shell::Shell() {
	DEBUG_PRINTF("TABLE_SIZE=%d", TABLE_SIZE);
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

uint16_t Shell::ValidateCmd(uint32_t nLength, CmdIndex &nCmdIndex) {
	uint16_t i;

	m_Argc = 0;

	for (i = 0; i < nLength; i++) {
		if ((m_Buffer[i] == ' ') || (m_Buffer[i] == '\t')) {
			m_Buffer[i] = '\0';
			break;
		}
	}

	for (uint32_t j = 0; j < TABLE_SIZE; j++) {
		if (0 == strcmp(m_Buffer, cmd_table[j].pName)) {
			nCmdIndex = static_cast<CmdIndex>(j);
			return static_cast<uint16_t>(i + 1);
		}
	}

	return 0;
}

void Shell::ValidateArg(uint16_t nOffset, uint32_t nLength) {
	if (nOffset > nLength) {
		return;
	}

	while (nOffset < nLength && ((m_Buffer[nOffset] == ' ') || (m_Buffer[nOffset] == '\t'))) {
		nOffset++;
	}

	if (nOffset == nLength) {
		return;
	}

	uint16_t nArgvStart = nOffset;
	m_Argv[0] = &m_Buffer[nOffset++];
	m_Argc = 1;

	uint16_t i, j = 1;

	for (i = nOffset; i < nLength; i++) {
		if ((m_Buffer[i] > ' ') && (m_Buffer[i] < 127)) {
			continue;
		}

		if ((m_Buffer[i] == ' ') || (m_Buffer[i] == '\t')) {
			if (j < MAXARG) {
				m_nArgvLength[j - 1] = static_cast<uint16_t>(i - nArgvStart);
			}
			while (i < nLength && ((m_Buffer[i] == ' ') || (m_Buffer[i] == '\t'))) {
				m_Buffer[i++] = '\0';
			}
			if (j < MAXARG) {
				nArgvStart = i;
				m_Argv[j++] = &m_Buffer[i];
			}
			m_Argc++;
		}
	}

	if (j < MAXARG) {
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
}

void Shell::Run() {	
	if (__builtin_expect((!m_bShownPrompt), 1)) {
		Puts(msg::CMD_PROMPT);
		m_bShownPrompt = true;
	}
	
	uint32_t nLength;

	if (__builtin_expect((ReadLine(nLength) == nullptr), 1)) {
		return;
	}
	
	Puts("\n");

	m_bShownPrompt = false; // next time round, we show the prompt.

	uint16_t nOffset;
	CmdIndex nCmdIndex;

	if ((nOffset = ValidateCmd(nLength, nCmdIndex)) == 0) {
		Printf("%s %s\n", msg::CMD_NOT_FOUND, m_Buffer);
		return;
	}

	ValidateArg(nOffset, nLength);

	if (m_Argc != cmd_table[static_cast<uint32_t>(nCmdIndex)].nArgc) {
		Puts(msg::CMD_WRONG_ARGUMENTS);
		return;
	}

	switch (nCmdIndex) {
		case CmdIndex::REBOOT:
			CmdReboot();
			break;
		case CmdIndex::INFO:
			CmdInfo();
			break;
		case CmdIndex::SET:
			CmdSet();
			break;
		case CmdIndex::GET:
			CmdGet();
			break;
		case CmdIndex::DHCP:
			CmdDhcp();
			break;
		case CmdIndex::DATE:
			CmdDate();
			break;
		case CmdIndex::HWCLOCK:
			CmdHwClock();
			break;
#ifndef NDEBUG
		case CmdIndex::I2CDETECT:
			CmdI2cDetect();
			break;
		case CmdIndex::DUMP:
			CmdDump();
			break;
		case CmdIndex::MEM:
			CmdMem();
			break;
		case CmdIndex::NTP:
			CmdNtp();
			break;
		case CmdIndex::GPS:
			CmdGps();
			break;
#endif
		case CmdIndex::HELP:
			CmdHelp();
			break;
		default:
			break;
	}
}
