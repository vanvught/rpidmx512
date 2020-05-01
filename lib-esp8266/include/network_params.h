/**
 * @file network_params.h
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef NETWORK_PARAMS_H_
#define NETWORK_PARAMS_H_

#include <stdint.h>
#include <stdbool.h>

#define ASSUME_ALIGNED  __attribute__((assume_aligned(4)))

#ifdef __cplusplus
extern "C" {
#endif

extern bool network_params_init(void);

extern bool network_params_is_use_dhcp(void);

extern uint32_t network_params_get_ip_address(void);
extern uint32_t network_params_get_net_mask(void);
extern uint32_t network_params_get_default_gateway(void);
extern uint32_t network_params_get_name_server(void);

extern /*@shared@*/const char *network_params_get_ssid(void) ASSUME_ALIGNED;
extern /*@shared@*/const char *network_params_get_password(void) ASSUME_ALIGNED;

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_PARAMS_H_ */
