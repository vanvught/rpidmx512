/**
 * @file poll.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <poll.h>

#include "debug.h"

extern int timerfd_poll(int fd, int timeout);

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
	int count = 0;

	for (unsigned i = 0; i < nfds; i++) {
		if (fds[i].fd < 0) {
			fds[i].revents = 0;
			continue;
		}

		switch (timerfd_poll(fds[i].fd, timeout)) {
		case -1:
			fds[i].revents = POLLERR;
			break;
		case 0:
			fds[i].revents = 0;
			break;
		default:
			fds[i].revents = POLLIN;
			break;
		}

		if (fds[i].revents != 0) {
			count++;
		}
	}

	return count;
}
