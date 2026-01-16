/**
 * @file dmxserial.h
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

#ifndef DMXSERIAL_H_
#define DMXSERIAL_H_

#include <cstdint>

#include "dmxserialchanneldata.h"
#include "dmxserialtftp.h"
#include "serial/serial.h"
#include "hal_uart.h"
#include "dmxnode.h"

namespace DmxSerialDefaults
{
inline constexpr auto TYPE = serial::Type::kUart;
inline constexpr auto UART_BAUD = 115200;
inline constexpr auto UART_BITS = hal::uart::BITS_8;
inline constexpr auto UART_PARITY = hal::uart::PARITY_NONE;
inline constexpr auto UART_STOPBITS = hal::uart::STOP_1BIT;
inline constexpr auto SPI_SPEED_HZ = 1000000; ///< 1 MHz
inline constexpr auto SPI_MODE = 0;
inline constexpr auto I2C_ADDRESS = 0x30;
inline constexpr auto I2C_SPEED_MODE = serial::i2c::Speed::kFast;
} // namespace DmxSerialDefaults

#define DMXSERIAL_FILE_PREFIX "chl"
#define DMXSERIAL_FILE_SUFFIX ".txt"

namespace DmxSerialFile
{
inline constexpr auto NAME_LENGTH = sizeof(DMXSERIAL_FILE_PREFIX "NNN" DMXSERIAL_FILE_SUFFIX) - 1;
inline constexpr auto MIN_NUMBER = 1;
inline constexpr auto MAX_NUMBER = dmxnode::kUniverseSize;
} // namespace DmxSerialFile

class DmxSerial final: public Serial
{
   public:
    DmxSerial();
    ~DmxSerial();

    void Init();

    void Start(uint32_t port_index);
    void Stop(uint32_t port_index);

    template <bool doUpdate> void SetData(uint32_t port_index, const uint8_t* pData, uint32_t length);

    void Sync(uint32_t port_index);
    void Sync();

    uint32_t GetUserData() { return 0; }    ///< Art-Net ArtPollReply
    uint32_t GetRefreshRate() { return 0; } ///< Art-Net ArtPollReply

    void Blackout([[maybe_unused]] bool blackout) {}
    void FullOn() {}

    bool SetDmxStartAddress([[maybe_unused]] uint16_t dmx_start_address) { return false; }
    uint16_t GetDmxStartAddress() { return dmxnode::kStartAddressDefault; }

    uint16_t GetDmxFootprint() { return dmxnode::kUniverseSize; }

    bool GetSlotInfo([[maybe_unused]] uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
    {
        slot_info.type = 0x00;       // ST_PRIMARY
        slot_info.category = 0x0001; // SD_INTENSITY
        return true;
    }

    void Print();

    uint32_t GetFilesCount() { return m_nFilesCount; }

    void EnableTFTP(bool enable_tftp);

    bool DeleteFile(int32_t file_number);
    bool DeleteFile(const char* file_number);

    void Input(const uint8_t* pBuffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    static bool FileNameCopyTo(char* file_name, uint32_t length, int32_t file_number);
    static bool CheckFileName(const char* file_name, int32_t& file_number);

    static DmxSerial* Get() { return s_this; }

   private:
    void ScanDirectory();
    void Update(const uint8_t* pData, uint32_t length);

    void static StaticCallbackFunction(const uint8_t* pBuffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(pBuffer, size, from_ip, from_port);
    }

   private:
    uint32_t m_nFilesCount{0};
    int32_t m_aFileIndex[DmxSerialFile::MAX_NUMBER];
    int32_t handle_{-1};
    DmxSerialChannelData* m_pDmxSerialChannelData[DmxSerialFile::MAX_NUMBER];
    uint16_t m_nDmxLastSlot{dmxnode::kUniverseSize};
    uint8_t dmx_data_[dmxnode::kUniverseSize];
    struct SyncData
    {
        uint8_t data[dmxnode::kUniverseSize];
        uint32_t length;
    };
    SyncData m_SyncData;
    bool enable_tftp_{false};
    DmxSerialTFTP* m_pDmxSerialTFTP{nullptr};

    static inline DmxSerial* s_this;
};

#endif  // DMXSERIAL_H_
