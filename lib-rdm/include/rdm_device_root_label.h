/**
 * @file rdm_device_root_label.h
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

#ifndef RDM_DEVICE_ROOT_LABEL_H_
#define RDM_DEVICE_ROOT_LABEL_H_

#if defined(H3)
#include "h3_board.h"
#elif defined(GD32)
#include "gd32_board.h"
#endif

namespace rdm::device
{
#if defined(CONFIG_RDM_DEVICE_ROOT_LABEL)
static constexpr char kRootLabel[] = CONFIG_RDM_DEVICE_ROOT_LABEL;
#else
#if defined(RDM_RESPONDER)
#if defined(H3)
static constexpr char kRootLabel[] = H3_BOARD_NAME " RDM Device";
#elif defined(GD32)
static constexpr char kRootLabel[] = GD32_BOARD_NAME " RDM Device";
#elif defined(RASPPI)
static constexpr char kRootLabel[] = "Raspberry Pi RDM Device";
#elif defined(__linux__)
static constexpr char kRootLabel[] = "Linux RDM Device";
#elif defined(__APPLE__)
static constexpr char kRootLabel[] = "MacOS RDM Device";
#else
static constexpr char kRootLabel[] = "RDM Device";
#endif
#else
static constexpr char kRootLabel[] = "RDMNet LLRP Only Device";
#endif
#endif
} // namespace rdm::device

#endif // RDM_DEVICE_ROOT_LABEL_H_
