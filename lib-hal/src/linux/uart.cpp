#if !defined(__clang__)	// Needed for compiling on MacOS
//FIXME Remove #pragma GCC push_options
# pragma GCC push_options
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
/**
 * @file uart.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cstring>
#include <glob.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "hal_uart.h"

#include "debug.h"

#if defined(__APPLE__)
 static constexpr char DEV[2][64] = {
		 { "/dev/tty.usbserial-*" },
		 { "/dev/tty.usbserial-*" }
 };
#else
 static constexpr char DEV[2][64] = {
		 { "/dev/ttyUSB*" },
		 { "/dev/ttyUSB*" },
 };
#endif

static int fd = -1;

static uint32_t s_nUart;
static uint32_t s_nBaudrate;
static uint32_t s_nBits;
static uint32_t s_nParity;
static uint32_t s_nStopBits;

static void s_configure_termios() {
	glob_t globbuf;

	if (s_nUart > sizeof(DEV) / sizeof(DEV[0])) {
		s_nUart = 0;
	}

	glob(DEV[s_nUart], 0, NULL, &globbuf);

	if (fd > 0) {
		close(fd);
	}

	if (globbuf.gl_pathc > 0) {
		fd = open(globbuf.gl_pathv[0], O_RDWR | O_NOCTTY | O_NDELAY);
	} else {
		fprintf(stderr, "No serial device found in %s\n", DEV[s_nUart]);
		return;
	}

	if (fd < 0) {
		printf("Error %i from open: %s\n", errno, strerror(errno));
		return;
	}

	printf("Using serial device at %s\n", globbuf.gl_pathv[0]);

	globfree(&globbuf);

	fcntl(fd, F_SETFL, 0);

	struct termios options;

	if (tcgetattr(fd, &options) != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		return;
	}

	if (s_nBaudrate == 115200) {
		cfsetispeed(&options, B115200);
		cfsetospeed(&options, B115200);
	} else {
		cfsetispeed(&options, B9600);
		cfsetospeed(&options, B9600);
	}

	options.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
	options.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	options.c_cflag &= ~CSIZE; // Clear all bits that set the data size
	options.c_cflag |= CS8; // 8 bits per byte (most common)
	options.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	options.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	options.c_lflag &= ~ICANON;
	options.c_lflag &= ~ECHO; // Disable echo
	options.c_lflag &= ~ECHOE; // Disable erasure
	options.c_lflag &= ~ECHONL; // Disable new-line echo
	options.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	options.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	options.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	options.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

	options.c_cc[VTIME] = 1;    // Wait for up to 1 deciseconds, returning as soon as any data is received.
	options.c_cc[VMIN] = 0;

	tcsetattr(fd, TCSANOW, &options);

	puts("\nDevice connected.");
}

void uart_begin(const uint32_t nUart, uint32_t nBaudrate, uint32_t nBits, uint32_t nParity, uint32_t nStopBits) {
	s_nUart = nUart;
	s_nBaudrate = nBaudrate;
	s_nBits = nBits;
	s_nParity = nParity;
	s_nStopBits = nStopBits;

	s_configure_termios();
}

void uart_set_baudrate(__attribute__((unused)) const uint32_t uart_base, uint32_t nBaudrate) {
	s_nBaudrate = nBaudrate;
	s_configure_termios();
}

void uart_transmit(__attribute__((unused)) const uint32_t uart_base, const uint8_t *pDate, uint32_t nLength) {
	if (fd < 0) {
		DEBUG_EXIT
		return;
	}

	const auto i = static_cast<uint32_t>(write(fd, pDate, nLength));

	if (i != nLength) {
		fprintf(stderr, "uart_transmit: bytes written %u [%u]\n", i, nLength);
	}
}

void uart_transmit_string(__attribute__((unused)) const uint32_t uart_base, const char *pData) {
	if (fd < 0) {
		DEBUG_EXIT
		return;
	}

	auto nLength = strlen(pData);
	const auto i = static_cast<uint32_t>(write(fd, pData, nLength));

	if (i != nLength) {
		fprintf(stderr, "uart_transmit_string: bytes written %u [%u]\n", i, static_cast<uint32_t>(nLength));
	}
}

uint32_t uart_get_rx_fifo_level(__attribute__((unused)) const uint32_t uart_base) {
	if (fd < 0) {
		DEBUG_EXIT
		return 0;
	}

	int32_t ret = 0;
    const auto i = ioctl(fd, FIONREAD, &ret);

    if (i <= 0) {
    	if (i == -1) {
    		perror("ioctl");
    	}
    	return 0;
    }

    return static_cast<uint32_t>(ret);
}

uint8_t uart_get_rx_data(__attribute__((unused)) const uint32_t uart_base) {
	if (fd < 0) {
		DEBUG_EXIT
		return ' ';
	}

	char c;

	const auto i = read(fd, &c, 1);

	if (i <= 0) {
		return static_cast<uint8_t>(' ');
	}

	return static_cast<uint8_t>(c);
}

uint32_t uart_get_rx(__attribute__((unused)) const uint32_t uart_base, char *pData, uint32_t nLength) {
	if (fd < 0) {
		DEBUG_EXIT
		return 0;
	}

	auto i = read(fd, pData, nLength);

	if (i <= 0) {
		return i;
	}

	return static_cast<uint32_t>(i);
}
