/**
 * @file artnet_params.h
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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


#ifndef ARTNET_PARAMS_H_
#define ARTNET_PARAMS_H_

#include <stdint.h>

typedef enum {
	OUTPUT_TYPE_DMX,
	OUTPUT_TYPE_SPI
} _output_type;


#ifdef __cplusplus
extern "C" {
#endif

extern void artnet_params_init(void);
extern const _output_type artnet_params_get_output_type(void);
extern const uint8_t artnet_params_get_net(void);
extern const uint8_t artnet_params_get_subnet(void);
extern const uint8_t artnet_params_get_universe(void);

#ifdef __cplusplus
}
#endif

#endif /* ARTNET_PARAMS_H_ */
