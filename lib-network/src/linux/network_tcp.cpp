#if !defined(CONFIG_NETWORK_USE_MINIMUM)
/**
 * @file network_tcp.cpp
 *
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifdef DEBUG_NETWORK
#undef NDEBUG
#endif

#include <cstdio>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>

#include "ip4/ip4_address.h"
#include "network_tcp.h"
#include "core/protocol/tcp.h"
#include "../../config/net_config.h"

namespace network::tcp {
// https://cboard.cprogramming.com/c-programming/158125-sockets-using-poll.html

#define MAX_PORTS_ALLOWED 2

struct PortInfo {
    CallbackListen callback;
    uint16_t nPort;
};

static PortInfo s_Ports[MAX_PORTS_ALLOWED];
static struct pollfd poll_set[MAX_PORTS_ALLOWED][TCP_MAX_TCBS_ALLOWED];
static int server_sockfd[MAX_PORTS_ALLOWED];
static uint8_t s_ReadBuffer[network::tcp::kDataSize];

bool Listen(uint16_t local_port, network::tcp::CallbackListen cb) {
    int32_t i;

    for (i = 0; i < MAX_PORTS_ALLOWED; i++) {
        if (s_Ports[i].nPort == local_port) {
            perror("Listen: connection already exists");
            return false;
        }

        if (s_Ports[i].nPort == 0) {
            break;
        }
    }

    if (i == MAX_PORTS_ALLOWED) {
        perror("Listen: too many connections");
        return false;
    }

    s_Ports[i].callback = cb;
    s_Ports[i].nPort = local_port;

    memset(&poll_set[i], 0, sizeof(poll_set[i]));

    server_sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sockfd[i] == -1) {
        perror("Could not create socket");
        return false;
    }

    int flag = 1;
    if (-1 == setsockopt(server_sockfd[i], SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
        perror("setsockopt fail");
    }

    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(local_port);

    if (bind(server_sockfd[i], (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("bind failed");
        printf(IPSTR ":%d\n", IP2STR(server.sin_addr.s_addr), local_port);
        return false;
    }

    listen(server_sockfd[i], TCP_MAX_TCBS_ALLOWED);

    for (int k = 0; k < TCP_MAX_TCBS_ALLOWED; k++) {
        poll_set[i][k].fd = -1;
        poll_set[i][k].events = 0;
        poll_set[i][k].revents = 0;
    }
    poll_set[i][0].fd = server_sockfd[i];
    poll_set[i][0].events = POLLIN | POLLPRI;

    printf("Listen -> i=%d\n", i);
    return true;
}

int32_t Send(ConnHandle conn_handle, const uint8_t* buffer, uint32_t length) {
    // Ignore SIGPIPE for this process
    signal(SIGPIPE, SIG_IGN);

    const int c = write(conn_handle, buffer, length);

    if (c < 0) {
        perror("write");
        return c;
    }

    return 0;
}

void Abort(ConnHandle connection_handle) {
    struct linger linger_option = {1, 0}; // Enable linger, zero timeout
    setsockopt(connection_handle, SOL_SOCKET, SO_LINGER, &linger_option, sizeof(linger_option));

    // Close the socket; this sends a TCP RST
    close(connection_handle);
}

void Run() {
    for (int32_t port_index = 0; port_index < MAX_PORTS_ALLOWED; port_index++) {
        // Skip unused ports (optional but helps)
        if (s_Ports[port_index].nPort == 0) {
            continue;
        }

        const int poll_result = poll(poll_set[port_index], TCP_MAX_TCBS_ALLOWED, 0);

        if (poll_result < 0) {
            perror("poll");
            continue; // don't kill the whole server
        }
        if (poll_result == 0) {
            continue; // no events on this port
        }

        for (int fd_index = 0; fd_index < TCP_MAX_TCBS_ALLOWED; fd_index++) {
            const short re = poll_set[port_index][fd_index].revents;
            if (re == 0) {
                continue;
            }

            int current_fd = poll_set[port_index][fd_index].fd;

            // handle error/hangup even if no POLLIN
            if (re & (POLLHUP | POLLERR | POLLNVAL)) {
                if (current_fd >= 0) {
                    close(current_fd);
                }
                poll_set[port_index][fd_index].fd = -1;
                poll_set[port_index][fd_index].events = 0;
                poll_set[port_index][fd_index].revents = 0;
                continue;
            }

            if (re & POLLIN) {
                if (current_fd == server_sockfd[port_index]) {
                    struct sockaddr_in client_address;
                    socklen_t client_len = sizeof(client_address);

                    const int client_sockfd = accept(server_sockfd[port_index], (struct sockaddr*)&client_address, &client_len);

                    if (client_sockfd < 0) {
                        perror("accept");
                        continue;
                    }

                    int empty_slot = -1;
                    for (int i = 1; i < TCP_MAX_TCBS_ALLOWED; i++) // start at 1, slot 0 is server
                    {
                        if (poll_set[port_index][i].fd < 0) // use -1 for empty
                        {
                            empty_slot = i;
                            break;
                        }
                    }

                    if (empty_slot == -1) {
                        close(client_sockfd);
                        continue;
                    }

                    poll_set[port_index][empty_slot].fd = client_sockfd;
                    poll_set[port_index][empty_slot].events = POLLIN | POLLPRI;
                } else {
                    int nread = 0;
                    ioctl(current_fd, FIONREAD, &nread);

                    if (nread == 0) {
                        close(current_fd);
                        poll_set[port_index][fd_index].fd = -1;
                        poll_set[port_index][fd_index].events = 0;
                        poll_set[port_index][fd_index].revents = 0;
                        continue;
                    }

                    const int bytes = read(current_fd, s_ReadBuffer, network::tcp::kDataSize);
                    if (bytes <= 0) {
                        close(current_fd);
                        poll_set[port_index][fd_index].fd = -1;
                        poll_set[port_index][fd_index].events = 0;
                        poll_set[port_index][fd_index].revents = 0;
                        continue;
                    }

                    if (s_Ports[port_index].callback != nullptr) {
                        // IMPORTANT: pass the FD, not fd_index
                        s_Ports[port_index].callback((ConnHandle)current_fd, (uint8_t*)s_ReadBuffer, (uint32_t)bytes);
                    }
                }
            }
        }
    }
}
#endif
} // namespace network::tcp
