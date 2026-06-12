/**
 * @file autodriver.h
 *
 */
/*
 * Based on https://github.com/sparkfun/L6470-AutoDriver/tree/master/Libraries/Arduino
 */
/* Copyright (C) 2017-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef AUTODRIVER_H_
#define AUTODRIVER_H_

#include <cstdint>

#include "l6470.h"

class AutoDriver final : public L6470 {
   public:
    AutoDriver(uint8_t, uint8_t, uint8_t, uint8_t);
    AutoDriver(uint8_t, uint8_t, uint8_t);

    ~AutoDriver() override;

    int busyCheck() override;

    void Print();

   private:
    uint8_t SPIXfer(uint8_t) override;

    /*
     * Additional methods
     */
   public:
    bool IsConnected();

    unsigned GetMotorNumber() { return motor_number_; }

    void SetMotorNumber(unsigned number) { motor_number_ = number; }

    static uint16_t GetNumBoards();
    static uint8_t GetNumBoards(uint8_t cs);

   private:
    uint8_t chip_select_;
    uint8_t reset_pin_;
    uint8_t busy_pin_;
    uint8_t position_;
    bool busy_;

    static uint8_t s_num_boards[2];
};

#endif /* AUTODRIVER_H_ */
