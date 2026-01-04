#if !defined(CONFIG_NETWORK_USE_MINIMUM)
/**
 * @file networktcp.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "net/tcp.h"
#include "net/ip4_address.h"
#include "../../config/net_config.h"

 #include "firmware/debug/debug_debug.h"

namespace net::tcp
{
// https://cboard.cprogramming.com/c-programming/158125-sockets-using-poll.html

#define MAX_PORTS_ALLOWED 2
#define MAX_SEGMENT_LENGTH 1400

struct PortInfo
{
    TcpCallbackFunctionPtr callback;
    uint16_t nPort;
};

static PortInfo s_Ports[MAX_PORTS_ALLOWED];
static struct pollfd poll_set[MAX_PORTS_ALLOWED][TCP_MAX_TCBS_ALLOWED];
static int server_sockfd[MAX_PORTS_ALLOWED];
static uint8_t s_ReadBuffer[MAX_SEGMENT_LENGTH];

int32_t Begin(uint16_t nLocalPort, [[maybe_unused]] TcpCallbackFunctionPtr callback)
{
    int32_t i;

    for (i = 0; i < MAX_PORTS_ALLOWED; i++)
    {
        if (s_Ports[i].nPort == nLocalPort)
        {
            perror("TcpBegin: connection already exists");
            return -2;
        }

        if (s_Ports[i].nPort == 0)
        {
            break;
        }
    }

    if (i == MAX_PORTS_ALLOWED)
    {
        perror("TcpBegin: too many connections");
        return -1;
    }

    s_Ports[i].callback = callback;
    s_Ports[i].nPort = nLocalPort;

    memset(&poll_set[i], 0, sizeof(poll_set[i]));

    server_sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sockfd[i] == -1)
    {
        perror("Could not create socket");
        return -1;
    }

    int flag = 1;
    if (-1 == setsockopt(server_sockfd[i], SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)))
    {
        perror("setsockopt fail");
    }

    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(nLocalPort);

    if (bind(server_sockfd[i], (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        perror("bind failed");
        printf(IPSTR ":%d\n", IP2STR(server.sin_addr.s_addr), nLocalPort);
        return -2;
    }

    listen(server_sockfd[i], TCP_MAX_TCBS_ALLOWED);

    poll_set[i][0].fd = server_sockfd[i];
    poll_set[i][0].events = POLLIN | POLLPRI;

    printf("Network::TcpBegin -> i=%d\n", i);
    return i;
}

int32_t End(const int32_t nHandle)
{
    assert(nHandle < MAX_PORTS_ALLOWED);

    close(poll_set[nHandle][0].fd);
    close(poll_set[nHandle][1].fd);

    return -1;
}

uint16_t Read(const int32_t nHandle, const uint8_t** ppBuffer, uint32_t& HandleConnectionIndex)
{
    assert(nHandle < MAX_PORTS_ALLOWED);

    const int poll_result = poll(poll_set[nHandle], TCP_MAX_TCBS_ALLOWED, 0);

    if (poll_result <= 0)
    {
        return poll_result;
    }

    for (int fd_index = 0; fd_index < TCP_MAX_TCBS_ALLOWED; fd_index++)
    {
        if (poll_set[nHandle][fd_index].revents & POLLIN)
        {
            int current_fd = poll_set[nHandle][fd_index].fd;
            if (current_fd != server_sockfd[nHandle])
            {
                int nread;
                ioctl(current_fd, FIONREAD, &nread);

                if (nread == 0)
                {
                    DEBUG_PRINTF("Removing client on fd %d", current_fd);
                    close(current_fd);
                    poll_set[nHandle][fd_index].fd = 0;
                    poll_set[nHandle][fd_index].events = 0;
                    poll_set[nHandle][fd_index].revents = 0;
                }
                else
                {
                    DEBUG_PRINTF("Serving client on fd %d", current_fd);
                    const int bytes = read(current_fd, s_ReadBuffer, MAX_SEGMENT_LENGTH);
                    if (bytes <= 0)
                    {
                        perror("read failed");
                        close(current_fd);
                        poll_set[nHandle][fd_index].fd = 0;
                        poll_set[nHandle][fd_index].events = 0;
                        poll_set[nHandle][fd_index].revents = 0;
                    }
                    else
                    {
                        HandleConnectionIndex = static_cast<uint32_t>(fd_index);
                        *ppBuffer = reinterpret_cast<uint8_t*>(&s_ReadBuffer);
                        return static_cast<uint16_t>(bytes);
                    }
                }
            }
        }
    }

    return 0;
}

void Write(const int32_t nHandle, const uint8_t* pBuffer, uint32_t nLength, const uint32_t HandleConnectionIndex)
{
    assert(nHandle < MAX_PORTS_ALLOWED);

    DEBUG_PRINTF("Write client on fd %d [%u]", poll_set[nHandle][HandleConnectionIndex].fd, HandleConnectionIndex);

    // Ignore SIGPIPE for this process
    signal(SIGPIPE, SIG_IGN);

    const int c = write(poll_set[nHandle][HandleConnectionIndex].fd, pBuffer, nLength);

    if (c < 0)
    {
        perror("write");
    }
}

static void close_with_rst(int socket_fd)
{
    struct linger linger_option = {1, 0}; // Enable linger, zero timeout
    setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &linger_option, sizeof(linger_option));

    // Close the socket; this sends a TCP RST
    close(socket_fd);
}

void Abort(const int32_t nHandle, const uint32_t HandleConnectionIndex)
{
    close_with_rst(poll_set[nHandle][HandleConnectionIndex].fd);
}

void Run()
{
    for (int32_t nHandle = 0; nHandle < MAX_PORTS_ALLOWED; nHandle++)
    {
        const int poll_result = poll(poll_set[nHandle], TCP_MAX_TCBS_ALLOWED, 0);

        if (poll_result <= 0)
        {
            return;
        }

        for (int fd_index = 0; fd_index < TCP_MAX_TCBS_ALLOWED; fd_index++)
        {
            if (poll_set[nHandle][fd_index].revents & POLLIN)
            {
                int current_fd = poll_set[nHandle][fd_index].fd;

                if (current_fd == server_sockfd[nHandle])
                {
                    struct sockaddr_in client_address;
                    int client_len = sizeof(struct sockaddr_in);

                    const int client_sockfd = accept(server_sockfd[nHandle], (struct sockaddr*)&client_address, (socklen_t*)&client_len);

                    if (client_sockfd < 0)
                    {
                        perror("accept failed");
                        return;
                    }

                    // Find an empty slot in poll_set to store the client's file descriptor
                    int empty_slot = -1;
                    for (int i = 0; i < 4; i++)
                    {
                        if (poll_set[nHandle][i].fd == 0)
                        {
                            empty_slot = i;
                            break;
                        }
                    }

                    if (empty_slot == -1)
                    {
                        // No empty slot found, handle the error
                        return;
                    }

                    poll_set[nHandle][empty_slot].fd = client_sockfd;
                    poll_set[nHandle][empty_slot].events = POLLIN | POLLPRI;

                    DEBUG_PRINTF("Adding client on fd %d", client_sockfd);
                }
                else
                {
                    int nread;
                    ioctl(current_fd, FIONREAD, &nread);
                    if (nread == 0)
                    {
                        DEBUG_PRINTF("Removing client on fd %d", current_fd);
                        close(current_fd);
                        poll_set[nHandle][fd_index].fd = 0;
                        poll_set[nHandle][fd_index].events = 0;
                        poll_set[nHandle][fd_index].revents = 0;
                    }
                    else
                    {
                        if (s_Ports[nHandle].callback != nullptr)
                        {
                            DEBUG_PRINTF("Serving client on fd %d", current_fd);
                            const int bytes = read(current_fd, s_ReadBuffer, MAX_SEGMENT_LENGTH);
                            if (bytes <= 0)
                            {
                                perror("read failed");
                                close(current_fd);
                                poll_set[nHandle][fd_index].fd = 0;
                                poll_set[nHandle][fd_index].events = 0;
                                poll_set[nHandle][fd_index].revents = 0;
                            }
                            else
                            {
                                s_Ports[nHandle].callback(static_cast<uint32_t>(fd_index), reinterpret_cast<uint8_t*>(&s_ReadBuffer), bytes);
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif
} // namespace net
