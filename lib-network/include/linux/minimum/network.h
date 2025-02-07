/**
 * @file network.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LINUX_MINIMUM_NETWORK_H_
#define LINUX_MINIMUM_NETWORK_H_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define MAX_SEGMENT_LENGTH		1400

typedef void (*UdpCallbackFunctionPtr)(const uint8_t *, uint32_t, uint32_t, uint16_t);

class Network {
public:
	static Network *Get() {
		static Network instance;
		return &instance;
	}

	const char *GetHostName() const {
		return m_aHostName;
	}

	int32_t Begin(const uint16_t nPort, [[maybe_unused]] UdpCallbackFunctionPtr callback = nullptr) {
		assert(func == nullptr);
		int nSocket;

		if ((nSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
			perror("socket");
			return -1;
		}

		int true_flag = true;

		if (setsockopt(nSocket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&true_flag), sizeof(int)) == -1) {
			perror("setsockopt(SO_BROADCAST)");
			return -1;
		}

		struct timeval recv_timeout;
		recv_timeout.tv_sec = 0;
		recv_timeout.tv_usec = 10;

		if (setsockopt(nSocket, SOL_SOCKET, SO_RCVTIMEO, static_cast<void*>(&recv_timeout), sizeof(recv_timeout)) == -1) {
			perror("setsockopt(SO_RCVTIMEO)");
			return -1;
		}

		int val = 1;
		if (setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
			perror("setsockopt(SO_REUSEADDR)");
			return -1;
		}

		val = 1;
		if (setsockopt(nSocket, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) == -1) {
			perror("setsockopt(SO_REUSEPORT)");
			return -1;
		}

		struct sockaddr_in si_me;
	    memset(&si_me, 0, sizeof(si_me));

	    si_me.sin_family = AF_INET;
	    si_me.sin_port = htons(nPort);
	    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(nSocket, reinterpret_cast<struct sockaddr*>(&si_me), sizeof(si_me)) == -1) {
			perror("bind");
			return -1;
		}

		return nSocket;
	}

	uint16_t RecvFrom(int32_t nHandle, const void **ppBuffer, uint32_t *pFromIp, uint16_t *pFromPort) {
		*ppBuffer = &m_buffer;
		return RecvFrom(nHandle, m_buffer, MAX_SEGMENT_LENGTH, pFromIp, pFromPort);
	}

	void SendTo(const int32_t nHandle, const void *pPacket, const uint16_t nLength, const uint32_t nToIp, const uint16_t nRemotePort) {
		struct sockaddr_in si_other;
		socklen_t slen = sizeof(si_other);

	    si_other.sin_family = AF_INET;
		si_other.sin_addr.s_addr = nToIp;
		si_other.sin_port = htons(nRemotePort);

		if (sendto(nHandle, pPacket, nLength, 0, reinterpret_cast<struct sockaddr*>(&si_other), slen) == -1) {
			perror("sendto");
		}
	}

	/*
	 * Not implemented
	 */

	void Print() {}

	/*
	 * UDP/IP
	 */

	int32_t End([[maybe_unused]] const uint16_t nPort) {
		return 0;
	}


private:
	Network() {
		memset(m_aHostName, 0, sizeof(m_aHostName));

		if (gethostname(m_aHostName, sizeof(m_aHostName)) < 0) {
			perror("gethostname");
		}
	}

	uint16_t RecvFrom(int32_t nHandle, void *pPacket, uint16_t nSize, uint32_t *pFromIp, uint16_t *pFromPort) {
		assert(pPacket != nullptr);
		assert(pFromIp != nullptr);
		assert(pFromPort != nullptr);

		int recv_len;
		struct sockaddr_in si_other;
		socklen_t slen = sizeof(si_other);


		if ((recv_len = recvfrom(nHandle, pPacket, nSize, 0, reinterpret_cast<struct sockaddr*>(&si_other), &slen)) == -1) {
			if (1 && (errno != EAGAIN) && (errno != EWOULDBLOCK)) { // EAGAIN and EWOULDBLOCK can be equal
				perror("recvfrom");
			}
			return 0;
		}

		*pFromIp = si_other.sin_addr.s_addr;
		*pFromPort = ntohs(si_other.sin_port);

		return static_cast<uint64_t>(recv_len);
	}

private:
	char m_aHostName[net::HOSTNAME_SIZE + 1];
	uint8_t m_buffer[MAX_SEGMENT_LENGTH];
};

#endif /* LINUX_MINIMUM_NETWORK_H_ */
