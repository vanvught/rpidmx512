#if defined(__linux__)
/**
 * @file ir_linux.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include "ir_linux.h"

#include "input.h"

IrLinux::IrLinux(void) : m_nFd(-1), m_PrevChar(INPUT_KEY_NOT_DEFINED) {

	memset(&m_Addr, '\0', sizeof(m_Addr));
	strcpy(m_Addr.sun_path, "/var/run/lirc/lircd");

	m_nFd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (m_nFd == -1) {
		perror("socket");
		return;
	}

	int flags = fcntl(m_nFd, F_GETFL, 0);

	if (flags < 0) {
		perror("fcntl");
		return;
	}

	flags |= O_NONBLOCK;

	if (fcntl(m_nFd, F_SETFL, flags) != 0) {
		perror("fcntl");
		return;
	}

	m_Addr.sun_family=AF_UNIX;

	if (connect(m_nFd, (struct sockaddr *) &m_Addr, sizeof(m_Addr)) == -1) {
		perror("connect");
		return;
	}
}

IrLinux::~IrLinux(void) {
	close(m_nFd);
}

bool IrLinux::IsAvailable(void) {
	const bool b = read(m_nFd, m_Buffer, sizeof(m_Buffer)) > 0 ? true : false;
	if (!b) {
		m_PrevChar = INPUT_KEY_NOT_DEFINED;
	}
	return b;
}

int IrLinux::GetChar(void) {
	char *p = strchr(m_Buffer,  '_');
	char *space = strchr(p, ' ');
	*space = '\0';
	int ch;

	puts(p);

	if (strcmp(p, "_OK") == 0) {
		ch = INPUT_KEY_ENTER;
	} else if (strcmp(p, "_NUMERIC_POUND") == 0) {
		ch = INPUT_KEY_ESC;
	} else if (strcmp(p, "_DOWN") == 0) {
		ch = INPUT_KEY_DOWN;
	} else if (strcmp(p, "_UP") == 0) {
		ch = INPUT_KEY_UP;
	} else if (strcmp(p, "_LEFT") == 0) {
		ch = INPUT_KEY_LEFT;
	} else if (strcmp(p, "_RIGHT") == 0) {
		ch = INPUT_KEY_RIGHT;
	} else {
		ch = INPUT_KEY_NOT_DEFINED;
	}

	if (m_PrevChar == ch) {
		return INPUT_KEY_NOT_DEFINED;
	}

	m_PrevChar = ch;

	return ch;

}

#endif
