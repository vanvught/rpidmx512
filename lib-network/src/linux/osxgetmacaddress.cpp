#if defined(__APPLE__)
/**
 * @file osxgetmacaddress.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>

#include "networklinux.h"

bool NetworkLinux::OSxGetMacaddress(const char *pIfName, uint8_t *pMacAddress) {
	int mib[6] = { CTL_NET, AF_ROUTE, 0, AF_LINK, NET_RT_IFLIST, 0 };
	size_t len;
	char *buf;

	if ((mib[5] = static_cast<int>(if_nametoindex(pIfName))) == 0) {
		perror("if_nametoindex error");
		return false;
	}

	if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
		perror("sysctl 1 error");
		return false;
	}

	if ((buf = (char *)malloc(len)) == NULL) {
		perror("malloc error");
		return false;
	}

	if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
		perror("sysctl 2 error");
		free(buf);
		return false;
	}

	const struct if_msghdr *ifm = (struct if_msghdr *) buf;
	const struct sockaddr_dl *sdl = (struct sockaddr_dl *) (ifm + 1);
	const unsigned char *ptr = (unsigned char *) LLADDR(sdl);
#ifndef NDEBUG
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4), *(ptr + 5));
#endif

	for(unsigned i = 0; i < 6;i++) {
		pMacAddress[i] = ptr[i];
	}

	free(buf);

	return true;
}
#endif

