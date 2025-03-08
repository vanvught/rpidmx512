/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "hardware.h"
#include "network.h"
#if !defined(NO_EMAC)
# include "networkconst.h"
#endif

#include "display.h"

#include "rdmresponder.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"
#if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
# include "rdmsubdevicesparams.h"
#endif

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "dmxnodechain.h"

#if !defined(NO_EMAC)
# include "remoteconfig.h"
# include "remoteconfigparams.h"
#endif

#include "flashcodeinstall.h"
#include "configstore.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "sparkfundmx.h"

int main() {
	Hardware hw;
	Display display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print();

	DmxNodeChain dmxNodeChain;

	SparkFunDmx sparkFunDmx;
	dmxNodeChain.SetSparkfunDmx(&sparkFunDmx);

	sparkFunDmx.ReadConfigFiles();

	auto nMotorsConnected = sparkFunDmx.GetMotorsConnected();
	auto isLedTypeSet = false;

	TLC59711DmxParams pwmledparms;
	pwmledparms.Load();

	TLC59711Dmx tlc59711Dmx;

	if ((isLedTypeSet = pwmledparms.IsSetLedType())) {
		dmxNodeChain.SetTLC59711Dmx(&tlc59711Dmx);
		display.Printf(7, "%s:%d", pwmledparms.GetType(pwmledparms.GetLedType()), pwmledparms.GetLedCount());
	}

	char aDescription[64];
	if (isLedTypeSet) {
		snprintf(aDescription, sizeof(aDescription) - 1, "Sparkfun [%d] with %s [%d]", nMotorsConnected, pwmledparms.GetType(pwmledparms.GetLedType()), pwmledparms.GetLedCount());
	} else {
		snprintf(aDescription, sizeof(aDescription) - 1, "Sparkfun [%d]", nMotorsConnected);
	}
	
	RDMPersonality *pRDMPersonalities[1] = { new  RDMPersonality(aDescription, &dmxNodeChain)};

	RDMResponder rdmResponder(pRDMPersonalities, 1);

	rdmResponder.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	rdmResponder.SetProductDetail(E120_PRODUCT_DETAIL_LED);

	RDMSensorsParams rdmSensorsParams;
	rdmSensorsParams.Load();
	rdmSensorsParams.Set();

#if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
	RDMSubDevicesParams rdmSubDevicesParams;
	rdmSubDevicesParams.Load();
	rdmSubDevicesParams.Set();
#endif

	rdmResponder.Init();

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&rdmResponder);

	rdmResponder.Start();
	rdmResponder.Print();

	hw.SetMode(hardware::ledblink::Mode::NORMAL);
	hal::watchdog_init();

	for (;;) {
		hal::watchdog_feed();
		rdmResponder.Run();
		hal::run();
	}
}
