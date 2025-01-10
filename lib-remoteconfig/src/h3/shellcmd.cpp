/**
 * @file shellcmd.cpp
 *
 */
/* Copyright (C) 2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>

#include "shell/shell.h"

#include "debug.h"

void h3_board_dump();

extern "C" {
void h3_dump_memory_mapping(void);
void h3_ccu_pll_dump(void);
void arm_dump_memmap(void);
void arm_dump_page_table(void);
}

namespace shell::dump {
namespace arg {
static constexpr char BOARD[] = "board";
static constexpr char MMAP[] = "mmap";
static constexpr char PLL[] = "pll";
static constexpr char LINKER[] = "linker";
}  // namespace arg
namespace length {
static constexpr auto BOARD = sizeof(arg::BOARD) - 1;
static constexpr auto MMAP = sizeof(arg::MMAP) - 1;
static constexpr auto PLL = sizeof(arg::PLL) - 1;
static constexpr auto LINKER = sizeof(arg::LINKER) - 1;
}  // namespace length
} // namespace shell::dump


using namespace shell;

void Shell::CmdDump() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == dump::length::BOARD) && (memcmp(m_Argv[0], dump::arg::BOARD, dump::length::BOARD) == 0)) {
		h3_board_dump();
		return;
	}

	if ((nArgv0Length == dump::length::MMAP) && (memcmp(m_Argv[0], dump::arg::MMAP, dump::length::MMAP) == 0)) {
		h3_dump_memory_mapping();
		return;
	}

	if ((nArgv0Length == dump::length::PLL) && (memcmp(m_Argv[0], dump::arg::PLL, dump::length::PLL) == 0)) {
		h3_ccu_pll_dump();
		return;
	}

	if ((nArgv0Length == dump::length::LINKER) && (memcmp(m_Argv[0], dump::arg::LINKER, dump::length::LINKER) == 0)) {
		arm_dump_memmap();
		return;
	}

	Puts(msg::error::INVALID);
}
