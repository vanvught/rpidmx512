/**
 * @file rdmsensorsparams.cpp
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

#ifdef DEBUG_RDMSENSORSPARAMS
#undef NDEBUG
#endif

#include <cstdint>

#include "json/rdmsensorsparams.h"
#include "json/json_parser.h"
#include "configstore.h"
#include "configurationstore.h"
 #include "firmware/debug/debug_debug.h"
#include "rdm_sensors.h"
#include "common/utils/utils_hex.h"
#include "rdmsensors.h"
#if !defined(CONFIG_RDM_SENSORS_DISABLE_BH170)
#include "rdmsensorbh1750.h"
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_MCP9808)
#include "rdmsensormcp9808.h"
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_HTU21D)
#include "rdmsensorhtu21dhumidity.h"
#include "rdmsensorhtu21dtemperature.h"
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_INA219)
#include "rdmsensorina219current.h"
#include "rdmsensorina219power.h"
#include "rdmsensorina219voltage.h"
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_SI7021)
#include "rdmsensorsi7021humidity.h"
#include "rdmsensorsi7021temperature.h"
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_THERMISTOR)
#include "rdmsensorthermistor.h"
#endif

namespace json
{

template <typename F> inline void ParseAddressArray(const char* val, size_t len, F&& callback)
{
    const auto* p = val;
    const auto* end = val + len;

    // Skip to '['
    while (p < end && *p != '[') ++p;
    if (p < end) ++p;

    while (p < end)
    {
        // Skip non-hex leading characters
        while (p < end && !(((*p >= '0') && (*p <= '9')) || ((*p >= 'a') && (*p <= 'f')) || ((*p >= 'A') && (*p <= 'F')))) ++p;

        if (p + 1 >= end) break; // Need at least two hex characters

        const auto kHi = common::hex::FromChar(*p++);
        const auto kLo = common::hex::FromChar(*p++);

        if (kHi != 0xFF && kLo != 0xFF)
        {
            const auto kByte = static_cast<uint8_t>((kHi << 4) | kLo);
            callback(kByte);
        }

        // Skip to next ',' or ']'
        while (p < end && *p != ',' && *p != ']') ++p;
        if (*p == ',')
        {
            ++p;
        }
        else if (*p == ']')
        {
            break;
        }
    }
}

RdmSensorsParams::RdmSensorsParams()
{
    ConfigStore::Instance().Copy(&store_rdmsensors, &ConfigurationStore::rdm_sensors);
}

void RdmSensorsParams::SetBH170(const char* val, uint32_t len)
{
    auto devices = store_rdmsensors.devices;
    if (devices == common::store::rdm::sensors::kMaxDevices) return;

    ParseAddressArray(val, len,
                      [&devices](uint8_t address)
                      {
                          DEBUG_PRINTF("%u:%x", devices, address);
                          store_rdmsensors.entry[devices].type = 0;
                          store_rdmsensors.entry[devices].address = address;
                          devices++;
                      });

    store_rdmsensors.devices = devices;
}

void RdmSensorsParams::SetHTU21D(const char* val, uint32_t len)
{
    auto devices = store_rdmsensors.devices;
    if (devices == common::store::rdm::sensors::kMaxDevices) return;

    ParseAddressArray(val, len,
                      [&devices](uint8_t address)
                      {
                          DEBUG_PRINTF("%u:%x", devices, address);
                          store_rdmsensors.entry[devices].type = 1;
                          store_rdmsensors.entry[devices].address = address;
                          devices++;
                      });

    store_rdmsensors.devices = devices;
}

void RdmSensorsParams::SetINA219(const char* val, uint32_t len)
{
    auto devices = store_rdmsensors.devices;
    if (devices == common::store::rdm::sensors::kMaxDevices) return;

    ParseAddressArray(val, len,
                      [&devices](uint8_t address)
                      {
                          DEBUG_PRINTF("%u:%x", devices, address);
                          store_rdmsensors.entry[devices].type = 2;
                          store_rdmsensors.entry[devices].address = address;
                          devices++;
                      });

    store_rdmsensors.devices = devices;
}

void RdmSensorsParams::SetMCP9808(const char* val, uint32_t len)
{
    auto devices = store_rdmsensors.devices;
    if (devices == common::store::rdm::sensors::kMaxDevices) return;

    ParseAddressArray(val, len,
                      [&devices](uint8_t address)
                      {
                          DEBUG_PRINTF("%u:%x", devices, address);
                          store_rdmsensors.entry[devices].type = 3;
                          store_rdmsensors.entry[devices].address = address;
                          devices++;
                      });

    store_rdmsensors.devices = devices;
}

void RdmSensorsParams::SetSI7021(const char* val, uint32_t len)
{
    auto devices = store_rdmsensors.devices;
    if (devices == common::store::rdm::sensors::kMaxDevices) return;

    ParseAddressArray(val, len,
                      [&devices](uint8_t address)
                      {
                          DEBUG_PRINTF("%u:%x", devices, address);
                          store_rdmsensors.entry[devices].type = 4;
                          store_rdmsensors.entry[devices].address = address;
                          devices++;
                      });

    store_rdmsensors.devices = devices;
}

void RdmSensorsParams::SetMCP3424(const char* val, uint32_t len)
{
    auto devices = store_rdmsensors.devices;
    if (devices == common::store::rdm::sensors::kMaxDevices) return;

    ParseAddressArray(val, len,
                      [&devices](uint8_t address)
                      {
                          DEBUG_PRINTF("%u:%x", devices, address);
                          store_rdmsensors.entry[devices].type = 5;
                          store_rdmsensors.entry[devices].address = address;
                          devices++;
                      });

    store_rdmsensors.devices = devices;
}

void RdmSensorsParams::Store(const char* buffer, uint32_t buffer_size)
{
    store_rdmsensors.devices = 0;
    ParseJsonWithTable(buffer, buffer_size, kRdmSensorsKeys);

    ConfigStore::Instance().Store(&store_rdmsensors, &ConfigurationStore::rdm_sensors);

#ifndef NDEBUG
    Dump();
#endif
}

static bool Add(RDMSensor* rdm_sensor)
{
    DEBUG_ENTRY();

    if (rdm_sensor->Initialize())
    {
        RDMSensors::Get()->Add(rdm_sensor);
        DEBUG_EXIT();
        return true;
    }

    delete rdm_sensor;

    DEBUG_EXIT();
    return false;
}

void RdmSensorsParams::Set()
{
    for (uint32_t i = 0; i < store_rdmsensors.devices; i++)
    {
        auto sensor_number = RDMSensors::Get()->GetCount();
        const auto kAddress = store_rdmsensors.entry[i].address;

        switch (static_cast<rdm::sensors::Types>(store_rdmsensors.entry[i].type))
        {
#if !defined(CONFIG_RDM_SENSORS_DISABLE_BH170)
            case rdm::sensors::Types::kBH170:
                Add(new RDMSensorBH170(sensor_number, kAddress));
                break;
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_HTU21D)
            case rdm::sensors::Types::kHTU21D:
                if (!Add(new RDMSensorHTU21DHumidity(sensor_number++, kAddress)))
                {
                    continue;
                }
                Add(new RDMSensorHTU21DTemperature(sensor_number, kAddress));
                break;
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_INA219)
            case rdm::sensors::Types::kINA219:
                if (!Add(new RDMSensorINA219Current(sensor_number++, kAddress)))
                {
                    continue;
                }
                if (!Add(new RDMSensorINA219Power(sensor_number++, kAddress)))
                {
                    continue;
                }
                Add(new RDMSensorINA219Voltage(sensor_number, kAddress));
                break;
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_MCP9808)
            case rdm::sensors::Types::kMCP9808:
                Add(new RDMSensorMCP9808(sensor_number, kAddress));
                break;
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_SI7021)
            case rdm::sensors::Types::kSI7021:
                if (!Add(new RDMSensorSI7021Humidity(sensor_number++, kAddress)))
                {
                    continue;
                }
                Add(new RDMSensorSI7021Temperature(sensor_number, kAddress));
                break;
#endif
#if !defined(CONFIG_RDM_SENSORS_DISABLE_THERMISTOR)
            case rdm::sensors::Types::kMCP3424:
                if (!Add(new RDMSensorThermistor(sensor_number, kAddress, 0, store_rdmsensors.calibrate[sensor_number])))
                {
                    continue;
                }
                sensor_number++;
                if (!Add(new RDMSensorThermistor(sensor_number, kAddress, 1, store_rdmsensors.calibrate[sensor_number])))
                {
                    continue;
                }
                sensor_number++;
                if (!Add(new RDMSensorThermistor(sensor_number, kAddress, 2, store_rdmsensors.calibrate[sensor_number])))
                {
                    continue;
                }
                sensor_number++;
                Add(new RDMSensorThermistor(sensor_number, kAddress, 3, store_rdmsensors.calibrate[sensor_number]));
                break;
#endif
            default:
                break;
        }
    }

#ifndef NDEBUG
    Dump();
#endif
}

void RdmSensorsParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::RdmSensorsParamsConst::kFileName);

    for (uint32_t i = 0; i < store_rdmsensors.devices; i++)
    {
        printf(" %s 0x%.2x\n", rdm::sensors::GetType(static_cast<rdm::sensors::Types>(store_rdmsensors.entry[i].type)), store_rdmsensors.entry[i].address);
    }

    for (uint32_t i = 0; i < common::store::rdm::sensors::kMaxSensors; i++)
    {
        printf("%2u %u\n", static_cast<unsigned int>(i), static_cast<unsigned int>(store_rdmsensors.calibrate[i]));
    }
}
} // namespace json