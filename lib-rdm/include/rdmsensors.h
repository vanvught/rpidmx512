/**
 * @file rdmsensors.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMSENSORS_H_
#define RDMSENSORS_H_

#include <cstdint>
#include <cassert>

#include "configurationstore.h"
#include "rdmsensor.h"
 #include "firmware/debug/debug_debug.h"

#if !defined(__APPLE__)
#define CONFIG_RDM_ENABLE_CPU_SENSOR
#endif

#if defined(NODE_RDMNET_LLRP_ONLY)
#undef CONFIG_RDM_ENABLE_SENSORS
#undef CONFIG_RDM_ENABLE_CPU_SENSOR
#endif

#if defined(CONFIG_RDM_ENABLE_CPU_SENSOR)
#include "sensor/cputemperature.h"
#endif
#if defined(CONFIG_RDM_ENABLE_SENSORS)
#include "json/rdmsensorsparams.h"
#endif

class RDMSensors
{
   public:
    RDMSensors()
    {
        DEBUG_ENTRY();
        assert(s_this == nullptr);
        s_this = this;

#if defined(CONFIG_RDM_ENABLE_SENSORS) || defined(CONFIG_RDM_ENABLE_CPU_SENSOR)
        rdm_sensor_ = new RDMSensor*[common::store::rdm::sensors::kMaxSensors];
        assert(rdm_sensor_ != nullptr);

#if defined(CONFIG_RDM_ENABLE_CPU_SENSOR)
        Add(new CpuTemperature(count_));
#endif
#if defined(CONFIG_RDM_ENABLE_SENSORS)
        json::RdmSensorsParams params;
        params.Load();
        params.Set();
#endif
#endif
        DEBUG_EXIT();
    }

    ~RDMSensors()
    {
        DEBUG_ENTRY();
        for (uint32_t i = 0; i < count_; i++)
        {
            if (rdm_sensor_[i] != nullptr)
            {
                delete rdm_sensor_[i];
                rdm_sensor_[i] = nullptr;
            }
        }

        delete[] rdm_sensor_;
        DEBUG_EXIT();
    }

    bool Add(RDMSensor* rdm_sensor)
    {
        DEBUG_ENTRY();

        if (rdm_sensor_ == nullptr)
        {
            DEBUG_EXIT();
            return false;
        }

        if (count_ == common::store::rdm::sensors::kMaxSensors)
        {
            DEBUG_EXIT();
            return false;
        }

        assert(rdm_sensor != nullptr);
        rdm_sensor_[count_++] = rdm_sensor;

        DEBUG_PRINTF("count_=%u", count_);
        DEBUG_EXIT();
        return true;
    }

    uint8_t GetCount() const { return count_; }

    const struct rdm::sensor::Defintion* GetDefintion(uint8_t sensor)
    {
        assert(sensor < count_);
        assert(rdm_sensor_[sensor] != nullptr);
        return rdm_sensor_[sensor]->GetDefintion();
    }

    const struct rdm::sensor::Values* GetValues(uint8_t sensor)
    {
        assert(sensor < count_);
        assert(rdm_sensor_[sensor] != nullptr);
        return rdm_sensor_[sensor]->GetValues();
    }

    void SetValues(uint8_t sensor)
    {
        if (sensor == 0xFF)
        {
            for (uint32_t i = 0; i < count_; i++)
            {
                rdm_sensor_[i]->SetValues();
            }
        }
        else
        {
            rdm_sensor_[sensor]->SetValues();
        }
    }

    void SetRecord(uint8_t sensor)
    {
        if (sensor == 0xFF)
        {
            for (uint32_t i = 0; i < count_; i++)
            {
                rdm_sensor_[i]->Record();
            }
        }
        else
        {
            rdm_sensor_[sensor]->Record();
        }
    }

    RDMSensor* GetSensor(uint8_t sensor) { return rdm_sensor_[sensor]; }

    static RDMSensors* Get() { return s_this; }

   private:
    RDMSensor** rdm_sensor_{nullptr};
    uint8_t count_{0};

    inline static RDMSensors* s_this;
};

#endif  // RDMSENSORS_H_
