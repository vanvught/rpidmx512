/**
 * @file net_config.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_CONFIG_H_
#define NET_CONFIG_H_

#if defined(__linux__) || defined (__APPLE__)
# define UDP_MAX_PORTS_ALLOWED			32
# define IGMP_MAX_JOINS_ALLOWED			(4 + (8 * 4)) /* 8 outputs x 4 Universes */
# define TCP_MAX_TCBS_ALLOWED			16
# define TCP_MAX_PORTS_ALLOWED			2
#else
# define TCP_MAX_PORTS_ALLOWED			1
# if defined (H3)
#  if !defined(HOST_NAME_PREFIX)
#   define HOST_NAME_PREFIX				"allwinner_"
#  endif
#  define UDP_MAX_PORTS_ALLOWED			16
#  define IGMP_MAX_JOINS_ALLOWED		(4 + (8 * 4)) /* 8 outputs x 4 Universes */
#  define TCP_MAX_TCBS_ALLOWED			16
# elif defined (GD32)
/*
 * Supports checking IPv4 header checksum and TCP, UDP, or ICMP checksum encapsulated in IPv4 or IPv6 datagram.
 */
#  define CHECKSUM_BY_HARDWARE
#  if !defined(HOST_NAME_PREFIX)
#   define HOST_NAME_PREFIX				"gigadevice_"
#  endif
#  if !defined (UDP_MAX_PORTS_ALLOWED)
#   define UDP_MAX_PORTS_ALLOWED		8
#  endif
#  if !defined (IGMP_MAX_JOINS_ALLOWED)
#   define IGMP_MAX_JOINS_ALLOWED		(4 + (8 * 4)) /* 8 outputs x 4 Universes */
#  endif
#  if !defined (TCP_MAX_TCBS_ALLOWED)
#   define TCP_MAX_TCBS_ALLOWED			6
#  endif
# else
#  error
# endif
#endif

#if !defined (UDP_MAX_PORTS_ALLOWED)
# error
#endif

#if !defined (IGMP_MAX_JOINS_ALLOWED)
# error
#endif

#if !defined (TCP_MAX_PORTS_ALLOWED)
# error
#endif

#if !defined (TCP_MAX_TCBS_ALLOWED)
# error
#endif

#endif /* NET_CONFIG_H_ */
