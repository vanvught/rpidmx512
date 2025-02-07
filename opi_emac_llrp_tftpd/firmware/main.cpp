/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if !defined (NODE_RDMNET_LLRP_ONLY)
# error
#endif

#include <cstdio>
#include <cstring>

#include "hardware.h"
#include "network.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "rdmnetllrponly.h"

#include "factorydefaults.h"

#include "firmwareversion.h"
#include "software_version.h"

namespace hal {
void reboot_handler() {
	if (!RemoteConfig::Get()->IsReboot()) {
		Display::Get()->SetSleep(false);
		Display::Get()->Cls();
		Display::Get()->TextStatus("Rebooting ...");
	}
}
}  // namespace hal

int main() {
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("RDMNet LLRP device only");

	RDMNetLLRPOnly device;
	device.Init();
	device.Print();

	RemoteConfig remoteConfig(remoteconfig::Node::RDMNET_LLRP_ONLY, remoteconfig::Output::CONFIG, 0);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	display.SetTitle("LLRP Only - TFTP");
	display.Set(2, displayudf::Labels::HOSTNAME);
	display.Set(3, displayudf::Labels::IP);
	display.Set(4, displayudf::Labels::DEFAULT_GATEWAY);
	display.Set(5, displayudf::Labels::VERSION);

	DisplayUdfParams displayUdfParams;
	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();

	display.Write(6, "mDNS enabled");
	display.TextStatus("Device running", CONSOLE_GREEN);

	hw.SetMode(hardware::ledblink::Mode::NORMAL);

	auto t1 = time(nullptr);
	struct tm tmHwClock;
	memset(&tmHwClock, 0, sizeof(struct tm));

	for (;;) {
		nw.Run();
		display.Run();
		hw.Run();

		time_t ltime;
		auto t2 = time(&ltime);
		if (t1 != t2) {
			t1 = t2;
			auto *tm = localtime(&ltime);
			struct tm tmlocal;
			memcpy(&tmlocal, tm, sizeof(struct tm));
			HwClock::Get()->Get(&tmHwClock);
			display.Printf(7, "%.2d:%.2d:%.2d %.2d:%.2d:%.2d",
					tmlocal.tm_hour, tmlocal.tm_min, tmlocal.tm_sec,
					tmHwClock.tm_hour, tmHwClock.tm_min, tmHwClock.tm_sec);
			printf("%.2d:%.2d:%.2d %.2d:%.2d:%.2d\r",
					tmlocal.tm_hour, tmlocal.tm_min, tmlocal.tm_sec,
					tmHwClock.tm_hour, tmHwClock.tm_min, tmHwClock.tm_sec);
		}

	}
}
