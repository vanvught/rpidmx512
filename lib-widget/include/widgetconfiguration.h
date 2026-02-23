/**
 * @file widgetconfiguration.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef WIDGETCONFIGURATION_H_
#define WIDGETCONFIGURATION_H_

#include <cstdint>

#include "widget.h"

#if defined(WIDGET_HAVE_FLASHROM)
#else
#include "../lib-hal/ff14b/source/ff.h"
#endif

#define DEVICE_TYPE_ID_LENGTH 2 ///<

enum
{
    WIDGET_MIN_BREAK_TIME = 9,
    WIDGET_DEFAULT_BREAK_TIME = 9,
    WIDGET_MAX_BREAK_TIME = 127
};

enum
{
    WIDGET_MIN_MAB_TIME = 1,
    WIDGET_DEFAULT_MAB_TIME = 1,
    WIDGET_MAX_MAB_TIME = 127,
};

enum
{
    WIDGET_DEFAULT_REFRESH_RATE = 40
};

typedef enum
{
    WIDGET_DEFAULT_FIRMWARE_LSB = 4 ///< x.4
} _firmware_version_lsb;

enum class Firmware
{
    kNormalDmx = 1, ///< Normal DMX firmware. Supports all messages except Send RDM (label=7), Send RDM Discovery Request(label=11) and receive RDM .
    kRdm = 2,       ///< RDM firmware. This enables the Widget to act as an RDM Controller.
    kRdmSniffer = 3 ///< RDM Sniffer firmware. This is for use with the Openlighting RDM packet monitoring application.
};

struct TWidgetConfiguration
{
    uint8_t firmware_lsb; ///< Firmware version LSB. Valid range is 0 to 255.
    uint8_t firmware_msb; ///< Firmware version MSB. Valid range is 0 to 255.
    uint8_t break_time;   ///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
    uint8_t mab_time;     ///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
    uint8_t refresh_rate; ///< DMX output rate in packets per second. Valid range is 1 to 40.
};

struct TWidgetConfigurationData
{
    uint8_t* data;
    uint8_t length;
};

struct WidgetConfiguration
{
    static void Get(struct TWidgetConfiguration* widget_configuration)
    {
        widget_configuration->break_time = s_break_time;
        widget_configuration->firmware_lsb = s_firmware_lsb;
        widget_configuration->firmware_msb = s_firmware_msb;
        widget_configuration->mab_time = s_mab_time;
        widget_configuration->refresh_rate = s_refresh_rate;
    }

    static void GetTypeId(struct TWidgetConfigurationData* info)
    {
        info->data = const_cast<uint8_t*>(s_device_type_id);
        info->length = DEVICE_TYPE_ID_LENGTH;
    }

    static void Store(const struct TWidgetConfiguration* widget_configuration);

    static void SetMode(widget::Mode mode);
    static void SetBreakTime(uint8_t break_time);
    static void SetMabTime(uint8_t mab_time);
    static void SetRefreshRate(uint8_t refresh_rate);
    static void SetThrottle(uint8_t throttle);

   private:
#if defined(WIDGET_HAVE_FLASHROM)
#else
    static void UpdateConfigFile();
    static void ProcessLineUpdate(const char* line, FIL* file_object_wr);
#endif

   private:
    inline static uint8_t s_device_type_id[DEVICE_TYPE_ID_LENGTH]{1, 0};
    inline static uint8_t s_firmware_lsb{WIDGET_DEFAULT_FIRMWARE_LSB};          ///< Firmware version LSB. Valid range is 0 to 255.
    inline static uint8_t s_firmware_msb{static_cast<uint8_t>(Firmware::kRdm)}; ///< Firmware version MSB. Valid range is 0 to 255.
    inline static uint8_t s_break_time{WIDGET_DEFAULT_BREAK_TIME};              ///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
    inline static uint8_t s_mab_time{WIDGET_DEFAULT_MAB_TIME}; ///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
    inline static uint8_t s_refresh_rate{WIDGET_DEFAULT_REFRESH_RATE}; ///< DMX output rate in packets per second. Valid range is 1 to 40.
};

#endif  // WIDGETCONFIGURATION_H_
