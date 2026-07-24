/**
 * @file rdm_debug.h
 *
 */
 /* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
 
#ifndef RDM_DEBUG_H_
#define RDM_DEBUG_H_

#ifdef DEBUG_RDM
#define RDM_DEBUG_ENTRY() DEBUG_ENTRY()
#define RDM_DEBUG_EXIT() DEBUG_EXIT()
#define RDM_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define RDM_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define RDM_DEBUG_ENTRY() \
    do {                     \
    } while (false)
#define RDM_DEBUG_EXIT() \
    do {                    \
    } while (false)
#define RDM_DEBUG_PRINTF(...) \
    do {                         \
    } while (false)
#define RDM_DEBUG_PUTS(...) \
    do {                       \
    } while (false)
#endif

#ifdef DEBUG_RDM_DEVICE_PARAMS
#define RDM_DEVICE_PARAMS_DEBUG_ENTRY() DEBUG_ENTRY()
#define RDM_DEVICE_PARAMS_DEBUG_EXIT() DEBUG_EXIT()
#define RDM_DEVICE_PARAMS_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define RDM_DEVICE_PARAMS_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define RDM_DEVICE_PARAMS_DEBUG_ENTRY() \
    do {                     \
    } while (false)
#define RDM_DEVICE_PARAMS_DEBUG_EXIT() \
    do {                    \
    } while (false)
#define RDM_DEVICE_PARAMS_DEBUG_PRINTF(...) \
    do {                         \
    } while (false)
#define RDM_DEVICE_PARAMS_DEBUG_PUTS(...) \
    do {                       \
    } while (false)
#endif

#ifdef DEBUG_RDM_DISCOVERY
#define RDM_DISCOVERY_DEBUG_ENTRY() DEBUG_ENTRY()
#define RDM_DISCOVERY_DEBUG_EXIT() DEBUG_EXIT()
#define RDM_DISCOVERY_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define RDM_DISCOVERY_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define RDM_DISCOVERY_DEBUG_ENTRY() \
    do {                     \
    } while (false)
#define RDM_DISCOVERY_DEBUG_EXIT() \
    do {                    \
    } while (false)
#define RDM_DISCOVERY_DEBUG_PRINTF(...) \
    do {                         \
    } while (false)
#define RDM_DISCOVERY_DEBUG_PUTS(...) \
    do {                       \
    } while (false)
#endif

#ifdef DEBUG_RDMNET
#define RDMNET_DEBUG_ENTRY() DEBUG_ENTRY()
#define RDMNET_DEBUG_EXIT() DEBUG_EXIT()
#define RDMNET_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define RDMNET_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define RDMNET_DEBUG_ENTRY() \
    do {                     \
    } while (false)
#define RDMNET_DEBUG_EXIT() \
    do {                    \
    } while (false)
#define RDMNET_DEBUG_PRINTF(...) \
    do {                         \
    } while (false)
#define RDMNET_DEBUG_PUTS(...) \
    do {                       \
    } while (false)
#endif

#ifdef DEBUG_RDM_LLRP
#define RDM_LLRP_DEBUG_ENTRY() DEBUG_ENTRY()
#define RDM_LLRP_DEBUG_EXIT() DEBUG_EXIT()
#define RDM_LLRP_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define RDM_LLRP_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define RDM_LLRP_DEBUG_ENTRY() \
    do {                     \
    } while (false)
#define RDM_LLRP_DEBUG_EXIT() \
    do {                    \
    } while (false)
#define RDM_LLRP_DEBUG_PRINTF(...) \
    do {                         \
    } while (false)
#define RDM_LLRP_DEBUG_PUTS(...) \
    do {                       \
    } while (false)
#endif

#ifdef DEBUG_LLRP_DEVICE
#define LLRP_DEVICE_DEBUG_ENTRY() DEBUG_ENTRY()
#define LLRP_DEVICE_DEBUG_EXIT() DEBUG_EXIT()
#define LLRP_DEVICE_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define LLRP_DEVICE_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define LLRP_DEVICE_DEBUG_ENTRY() \
    do {                     \
    } while (false)
#define LLRP_DEVICE_DEBUG_EXIT() \
    do {                    \
    } while (false)
#define LLRP_DEVICE_DEBUG_PRINTF(...) \
    do {                         \
    } while (false)
#define LLRP_DEVICE_DEBUG_PUTS(...) \
    do {                       \
    } while (false)
#endif

#endif // RDM_DEBUG_H_
