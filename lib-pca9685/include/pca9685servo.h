/**
 * @file pca9685servo.h
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PCA9685SERVO_H_
#define PCA9685SERVO_H_

#include <cstdint>

#include "pca9685.h"

namespace pca9685::servo
{
inline constexpr uint32_t kLeftDefaultUs = 1000;
inline constexpr uint32_t kCenterDefaultUs = 1500;
inline constexpr uint32_t kRightDefaultUs = 2000;
} // namespace pca9685::servo

class PCA9685Servo : public PCA9685
{
   public:
    explicit PCA9685Servo(uint8_t address) : PCA9685(address)
    {
        SetInvert(pca9685::Invert::kOutputNotInverted);
        SetOutDriver(pca9685::Output::kDriverTotempole);
        SetFrequency(50);
        CalcLeftCount();
        CalcRightCount();
        CalcCenterCount();
    }

    void SetLeftUs(uint16_t nLeftUs)
    {
        if ((nLeftUs < m_nRightUs) && (nLeftUs < m_nCenterUs))
        {
            m_nLeftUs = nLeftUs;
            CalcLeftCount();
        }
    }

    uint16_t GetLeftUs() const { return m_nLeftUs; }

    void SetRightUs(uint16_t nRightUs)
    {
        if ((m_nLeftUs < nRightUs) && (m_nCenterUs < nRightUs))
        {
            m_nRightUs = nRightUs;
            CalcRightCount();
        }
    }

    uint16_t GetRightUs() const { return m_nRightUs; }

    void SetCenterUs(uint16_t nCenterUs)
    {
        if ((nCenterUs < m_nRightUs) && (m_nLeftUs < nCenterUs))
        {
            m_nCenterUs = nCenterUs;
            CalcCenterCount();
        }
    }

    uint16_t GetCenterUs() const { return m_nCenterUs; }

    void Set(uint32_t channel, uint16_t data)
    {
        if (data > right_count_)
        {
            data = right_count_;
        }
        else if (data < left_count_)
        {
            data = left_count_;
        }

        Write(channel, data);
    }

    void Set(uint32_t channel,  uint8_t data)
    {
        if (data == 0)
        {
            Write(channel, left_count_);
        }
        else if (data == (0xFF + 1) / 2)
        {
            Write(channel, center_count_);
        }
        else if (data == 0xFF)
        {
            Write(channel, right_count_);
        }
        else
        {
            const auto kCount = static_cast<uint16_t>(left_count_ + (.5f + (static_cast<float>((right_count_ - left_count_)) / 0xFF) * data));
            Write(channel, kCount);
        }
    }

    void SetAngle(uint32_t channel,  uint8_t nAngle)
    {
        if (nAngle == 0)
        {
            Write(channel, left_count_);
        }
        else if (nAngle == 90)
        {
            Write(channel, center_count_);
        }
        else if (nAngle >= 180)
        {
            Write(channel, right_count_);
        }
        else if (nAngle < 90)
        {
            const auto kCount = static_cast<uint16_t>(left_count_ + (.5f + (static_cast<float>((center_count_ - left_count_)) / 90) * nAngle));
            Write(channel, kCount);
        }
        else
        {
            const auto kCount =
                static_cast<uint16_t>((2.0f * center_count_) - right_count_ + (.5f + (static_cast<float>((right_count_ - center_count_)) / 90) * nAngle));
            Write(channel, kCount);
        }
    }

   private:
    void CalcLeftCount() { left_count_ = static_cast<uint16_t>((.5f + ((204.8f * m_nLeftUs) / 1000U))); }

    void CalcRightCount() { right_count_ = static_cast<uint16_t>((.5f + ((204.8f * m_nRightUs) / 1000U))); }

    void CalcCenterCount() { center_count_ = static_cast<uint16_t>((.5f + ((204.8f * m_nCenterUs) / 1000U))); }

   private:
    uint16_t m_nLeftUs{pca9685::servo::kLeftDefaultUs};
    uint16_t m_nRightUs{pca9685::servo::kRightDefaultUs};
    uint16_t m_nCenterUs{pca9685::servo::kCenterDefaultUs};
    uint16_t left_count_;
    uint16_t right_count_;
    uint16_t center_count_;
};

#endif  // PCA9685SERVO_H_
