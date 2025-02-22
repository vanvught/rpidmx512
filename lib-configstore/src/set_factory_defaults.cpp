/**
 * @file set_factory_defaults.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_CONFIGSTORE)
# undef NDEBUG
#endif

#include "configstore.h"
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
#include "dmxnode_nodetype.h"
#endif
#include "debug.h"

namespace configstore {
void set_factory_defaults() {
	DEBUG_ENTRY

	ConfigStore::Get()->ResetStore(configstore::Store::RCONFIG);
	ConfigStore::Get()->ResetStore(configstore::Store::NETWORK);
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
#if defined (DISPLAY_UDF)
	ConfigStore::Get()->ResetStore(configstore::Store::DISPLAYUDF);
#endif
#if defined (DMXNODE_TYPE_ARTNETNODE) ||  defined (DMXNODE_TYPE_E131BRIDGE)
	ConfigStore::Get()->ResetStore(configstore::Store::NODE);
#endif
#if defined (NODE_LTC_SMPTE)
	ConfigStore::Get()->ResetStore(configstore::Store::LTC);
	ConfigStore::Get()->ResetStore(configstore::Store::LTCDISPLAY);
	ConfigStore::Get()->ResetStore(configstore::Store::LTCETC);
	ConfigStore::Get()->ResetStore(configstore::Store::TCNET);
	ConfigStore::Get()->ResetStore(configstore::Store::GPS);
#endif
#if defined (NODE_OSC_SERVER)
	ConfigStore::Get()->ResetStore(configstore::Store::OSC);
#endif
#if defined (NODE_OSC_CLIENT)
	ConfigStore::Get()->ResetStore(configstore::Store::OSC_CLIENT);
#endif
#if defined (NODE_SHOWFILE)
	ConfigStore::Get()->ResetStore(configstore::Store::SHOW);
#endif
#if defined (RDM_RESPONDER)
	ConfigStore::Get()->ResetStore(configstore::Store::RDMDEVICE);
	ConfigStore::Get()->ResetStore(configstore::Store::RDMSENSORS);
# if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
	ConfigStore::Get()->ResetStore(configstore::Store::RDMSUBDEVICES);
# endif
#endif

#if defined (DMXNODE_OUTPUT_DMX)
	ConfigStore::Get()->ResetStore(configstore::Store::DMXSEND);
#endif
#if defined (DMXNODE_OUTPUT_PIXEL) || defined(OUTPUT_DMX_TLC59711)
	ConfigStore::Get()->ResetStore(configstore::Store::WS28XXDMX);
#endif
#if defined (OUTPUT_DMX_MONITOR)
	ConfigStore::Get()->ResetStore(configstore::Store::MONITOR);
#endif
#if defined (OUTPUT_DMX_SERIAL)
	ConfigStore::Get()->ResetStore(configstore::Store::SERIAL);
#endif
#if defined (OUTPUT_RGB_PANEL)
	ConfigStore::Get()->ResetStore(configstore::Store::RGBPANEL);
#endif
#if defined (OUTPUT_DMX_PCA9685)
	ConfigStore::Get()->ResetStore(configstore::Store::PCA9685);
#endif
#if defined(OUTPUT_DMX_STEPPER)
	ConfigStore::Get()->ResetStore(configstore::Store::SPARKFUN);
	ConfigStore::Get()->ResetStore(configstore::Store::MOTORS);
#endif
#endif

	DEBUG_EXIT
}
}  // namespace configstore
