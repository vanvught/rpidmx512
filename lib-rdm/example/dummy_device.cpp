/**
 * @file dummy_device.cpp
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

#include "hal.h"
#include "network.h"
#include "configstore.h"
#include "firmwareversion.h"
#include "software_version.h"
#include "rdmdevice.h"
#include "rdmnetdevice.h"

namespace rdm::device::responder
{
void SetFactoryDefaults() {}
} // namespace rdm::device::responder

int main(int argc, char** argv)
{
    hal::Init();
    ConfigStore config_store;
    Network nw(argc, argv);
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

    hal::print();
    fw.Print();
    nw.Print();

    auto& rdm_device = rdm::device::Device::Instance();
    rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
    rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_LED);

    RDMNetDevice device;
	device.Print();
	
    for (;;)
    {
    }

    return 0;
}
