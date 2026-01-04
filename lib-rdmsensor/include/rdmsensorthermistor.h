/**
 * @file rdmsensorthermistor.h
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMSENSORTHERMISTOR_H_
#define RDMSENSORTHERMISTOR_H_

#include <cstdint>

#include "rdmsensor.h"
#include "rdmsensorsstore.h"
#include "rdm_e120.h"

#include "mcp3424.h"
#include "thermistor.h"

 #include "firmware/debug/debug_debug.h"

class RDMSensorThermistor final : public RDMSensor, MCP3424
{
   public:
    explicit RDMSensorThermistor(uint8_t sensor, uint8_t address = 0, uint8_t channel = 0, int32_t calibration = 0)
        : RDMSensor(sensor), MCP3424(address), calibration_(calibration), channel_(channel)
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("nSensor=%u, address=0x%.2x, channel=%u, nCalibration=%d", sensor, address, channel, calibration);

        SetType(E120_SENS_TEMPERATURE);
        SetUnit(E120_UNITS_CENTIGRADE);
        SetPrefix(E120_PREFIX_NONE);
        SetRangeMin(rdm::sensor::SafeRangeMin(sensor::thermistor::RANGE_MIN));
        SetRangeMax(rdm::sensor::SafeRangeMax(sensor::thermistor::RANGE_MAX));
        SetNormalMin(rdm::sensor::SafeRangeMin(sensor::thermistor::RANGE_MIN));
        SetNormalMax(rdm::sensor::SafeRangeMax(sensor::thermistor::RANGE_MAX));
        SetDescription(sensor::thermistor::DESCRIPTION);

        DEBUG_EXIT();
    }

    bool Initialize() override { return MCP3424::IsConnected(); }

    bool Calibrate(float f)
    {
        const auto kCalibrate = static_cast<int32_t>(f * 10);
        uint32_t resistor;
        const auto kMeasure = static_cast<int32_t>(GetValue(resistor) * 10);

        DEBUG_PRINTF("kCalibrate=%d, kMeasure=%d", kCalibrate, kMeasure);

        if (kCalibrate == kMeasure)
        {
            return true;
        }

        int32_t offset = 10;

        if (kCalibrate > kMeasure)
        {
            offset = -10;
        }

        for (int32_t i = 1; i < 128; i++)
        {
            calibration_ = i * offset;
            const auto kMeasures = static_cast<int32_t>(GetValue(resistor) * 10);
            DEBUG_PRINTF("kCalibrate=%d, kMeasures=%d, m_nCalibration=%d, resistor=%u", kCalibrate, kMeasures, calibration_, resistor);
            if (kCalibrate == kMeasures)
            {
                rdmsensors_store::SaveCalibration(RDMSensor::GetSensor(), calibration_);
                return true;
            }
        }

        return false;
    }

    void ResetCalibration()
    {
        calibration_ = 0;
        rdmsensors_store::SaveCalibration(RDMSensor::GetSensor(), calibration_);
    }

    int32_t GetCalibration() const { return calibration_; }

    float GetValue(uint32_t& resistor)
    {
        double sum = 0;
        for (uint32_t i = 0; i < 4; i++)
        {
            const auto kV = MCP3424::GetVoltage(channel_);
            sum += kV;
        }
        const auto kV = sum / 4;
        const auto kR = Resistor(kV);
        const auto kT = sensor::thermistor::Temperature(kR);
        DEBUG_PRINTF("v=%1.3f, r=%u, t=%3.1f", kV, kR, kT);
        resistor = kR;
        return kT;
    }

    int16_t GetValue() override
    {
        uint32_t resistor;
        return static_cast<int16_t>(GetValue(resistor));
    }

   private:
    int32_t calibration_;
    uint8_t channel_;

    /*
     * The R values are based on:
     * https://www.abelectronics.co.uk/p/69/adc-pi-raspberry-pi-analogue-to-digital-converter
     */
    static constexpr int32_t kRGnd = 6800;   // 6K8
    static constexpr int32_t kRHigh = 10000; // 10K

    uint32_t Resistor(double vin)
    {
        const double kD = (5 * kRGnd) / vin;
        const auto kR = static_cast<int32_t>(kD) - kRGnd - kRHigh + calibration_;
        return static_cast<uint32_t>(kR);
    }
};

#endif  // RDMSENSORTHERMISTOR_H_
