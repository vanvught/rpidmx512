/**
 * @file showfileprotocol.h
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

#ifndef SHOWFILEPROTOCOL_H_
#define SHOWFILEPROTOCOL_H_

#if defined (CONFIG_SHOWFILE_PROTOCOL_E131)
# include "protocols/showfileprotocole131.h"
#elif defined (CONFIG_SHOWFILE_PROTOCOL_ARTNET)
# include "protocols/showfileprotocolartnet.h"
#elif defined (CONFIG_SHOWFILE_PROTOCOL_NODE_E131)
# define CONFIG_SHOWFILE_PROTOCOL_INTERNAL
# include "protocols/showfileprotocolnodee131.h"
#elif defined (CONFIG_SHOWFILE_PROTOCOL_NODE_ARTNET)
# define CONFIG_SHOWFILE_PROTOCOL_INTERNAL
# include "protocols/showfileprotocolnodeartnet.h"
#else
# error Protocol is not supported
#endif

#if defined(CONFIG_SHOWFILE_PROTOCOL_E131) && defined(CONFIG_SHOWFILE_PROTOCOL_ARTNET)
# error Protocol configuration error
#endif

#if defined(CONFIG_SHOWFILE_PROTOCOL_NODE_E131) && defined(CONFIG_SHOWFILE_PROTOCOL_NODE_ARTNET)
# error Protocol node configuration error
#endif

#endif /* SHOWFILEPROTOCOL_H_ */
