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

#include <cstdio>

#include "h3/hal.h"
#include "h3/hal_watchdog.h"
#include "display.h"
#include "rdmresponder.h"
#include "rdmpersonality.h"
#include "tlc59711.h"
#include "json/tlc59711dmxparams.h"
#include "tlc59711dmx.h"
#include "dmxnodechain.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "firmwareversion.h"
#include "software_version.h"
#include "sparkfundmx.h"
#include "common/utils/utils_enum.h"
#include "configurationstore.h"
#if !defined(NO_EMAC)
#include "networkconst.h"
#include "remoteconfig.h"
#include "network.h"
#endif

namespace hal
{
void RebootHandler() {}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    Display display;
    ConfigStore config_store;
#if !defined(NO_EMAC)    
    NetworkInit();
#endif    
    FirmwareVersion fw(kSoftwareVersion, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print();

    DmxNodeChain dmxNodeChain;

    SparkFunDmx sparkFunDmx;
    dmxNodeChain.SetSparkfunDmx(&sparkFunDmx);

    sparkFunDmx.ReadConfigFiles();

    const auto kMotorsConnected = sparkFunDmx.GetMotorsConnected();
 
    TLC59711Dmx tlc59711dmx;

    json::Tlc59711DmxParams pwmledparms;
    pwmledparms.Load();
    pwmledparms.Set();

    const auto kType = common::FromValue<tlc59711::Type>(ConfigStore::Instance().DmxLedGet(&common::store::DmxLed::type));
    const auto kIsLedTypeSet = kType != tlc59711::Type::kUndefined;

    char description[64];

    if (kIsLedTypeSet)
    {
        dmxNodeChain.SetTLC59711Dmx(&tlc59711dmx);
        display.Printf(7, "%s:%d", tlc59711::GetType(tlc59711dmx.GetType()), tlc59711dmx.GetCount());

        snprintf(description, sizeof(description) - 1, "Sparkfun [%d] with %s [%d]", kMotorsConnected, tlc59711::GetType(tlc59711dmx.GetType()),
                 tlc59711dmx.GetCount());
    }
    else
    {
        snprintf(description, sizeof(description) - 1, "Sparkfun [%d]", kMotorsConnected);
    }
	
    RDMPersonality* rdm_personalities[1] = {new RDMPersonality(description, &dmxNodeChain)};

    RDMResponder rdm_responder(rdm_personalities, 1);
    rdm_responder.Init();
    rdm_responder.Start();
    rdm_responder.Print();

    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        rdm_responder.Run();
        hal::Run();
    }
}
