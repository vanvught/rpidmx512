/*
 * network.cpp
 *
 * Should move to lib-network
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <cassert>

#include "network.h"

#define MAX_PORTS_ALLOWED		2
#define MAX_SEGMENT_LENGTH		1400

static uint16_t s_ports_allowed[MAX_PORTS_ALLOWED];
static struct pollfd pollfds[MAX_PORTS_ALLOWED][2];
static uint8_t s_ReadBuffer[MAX_SEGMENT_LENGTH];

int32_t Network::TcpBegin(uint16_t nLocalPort) {
	int32_t i;

	for (i = 0; i < MAX_PORTS_ALLOWED; i++) {
		if (s_ports_allowed[i] == nLocalPort) {
			perror("TcpBegin: connection already exists");
			return -2;
		}

		if (s_ports_allowed[i] == 0) {
			break;
		}
	}

	if (i == MAX_PORTS_ALLOWED) {
		perror("TcpBegin: too many connections");
		return -1;
	}

	s_ports_allowed[i] = nLocalPort;

	memset(&pollfds[i], 0, sizeof(pollfds));

	int serverfd = socket(AF_INET, SOCK_STREAM, 0);

	if (serverfd == -1) {
		perror("Could not create socket");
		return -1;
	}

    int flag = 1;
    if (-1 == setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
        perror("setsockopt fail");
    }

	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(nLocalPort);

	if (bind(serverfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
		perror("bind failed");
		return -2;
	}

	listen(serverfd, 0);

    pollfds[i][0].fd = serverfd;
    pollfds[i][0].events = POLLIN | POLLPRI;

    printf("Network::TcpBegin -> i=%d\n", i);
	return i;
}

int32_t Network::TcpEnd(const int32_t nHandle) {
	assert(nHandle < MAX_PORTS_ALLOWED);

	close(pollfds[nHandle][0].fd);
	close(pollfds[nHandle][1].fd);

	return -1;
}

uint16_t Network::TcpRead(const int32_t nHandle, uint8_t **ppBuffer) {
	assert(nHandle < MAX_PORTS_ALLOWED);

	const int poll_result = poll(pollfds[nHandle], 2, 0);

	if (poll_result <= 0) {
		return poll_result;
	}

	if (pollfds[nHandle][0].revents & POLLIN) {
		struct sockaddr_in client;
		int c = sizeof(struct sockaddr_in);

		const int clientfd = accept(pollfds[nHandle][0].fd, (struct sockaddr*) &client, (socklen_t*) &c);

		if (clientfd < 0) {
			perror("accept failed");
			return clientfd;
		}

		pollfds[nHandle][1].fd = clientfd;
		pollfds[nHandle][1].events = POLLIN | POLLPRI;

	}

	if ((pollfds[nHandle][1].fd > 0) && (pollfds[nHandle][1].revents & POLLIN)) {
		const int bytes = read(pollfds[nHandle][1].fd, s_ReadBuffer, MAX_SEGMENT_LENGTH);

		if (bytes <= 0) {
			perror("read failed");
			pollfds[nHandle][1].fd = 0;
			pollfds[nHandle][1].events = 0;
			pollfds[nHandle][1].revents = 0;
		} else {
			*ppBuffer = reinterpret_cast<uint8_t*>(&s_ReadBuffer);
			return static_cast<uint16_t>(bytes);
		}
	}

	return 0;
}

void Network::TcpWrite(const int32_t nHandle, const uint8_t *pBuffer, uint16_t nLength) {
	assert(nHandle < MAX_PORTS_ALLOWED);

	const int c = write(pollfds[nHandle][1].fd, pBuffer, nLength);

	if (c < 0) {
		perror("write");
	}
}
