/**
 * @file dmxserial.cpp
 *
 */
/* Copyright (C) 2020-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>
#include <cerrno>
#include <cassert>

#include "dmxserial.h"
#include "core/protocol/udp.h"
#include "dmxserial_internal.h"
#include "dmxserialtftp.h"
#include "network_udp.h"
#include "board.h"
#include "firmware/debug/debug_debug.h"

DmxSerial::DmxSerial() {
    assert(s_this == nullptr);
    s_this = this;

    for (uint32_t i = 0; i < DmxSerialFile::MAX_NUMBER; i++) {
        m_aFileIndex[i] = -1;
        m_pDmxSerialChannelData[i] = nullptr;
    }

    memset(dmx_data_, 0, sizeof(dmx_data_));
}

DmxSerial::~DmxSerial() {
    for (uint32_t i = 0; i < files_count_; i++) {
        if (m_pDmxSerialChannelData[i] != nullptr) {
            delete m_pDmxSerialChannelData[i];
        }
    }

    network::udp::End(UDP::PORT);
    handle_ = -1;

    s_this = nullptr;
}

void DmxSerial::Init() {
    assert(handle_ == -1);
    handle_ = network::udp::Begin(UDP::PORT, StaticCallbackFunction);
    assert(handle_ != -1);

    ScanDirectory();
    Serial::Init();
}

void DmxSerial::Start([[maybe_unused]] uint32_t port_index) {
    // No actions here
}

void DmxSerial::Stop([[maybe_unused]] uint32_t port_index) {
    // No actions here
}

template <bool doUpdate> void DmxSerial::SetData([[maybe_unused]] uint32_t port_index, const uint8_t* data, uint32_t length) {
    assert(port_index == 0);
    assert(data != nullptr);

    if constexpr (doUpdate) {
        Update(data, length);
        return;
    }

    memcpy(m_SyncData.data, data, length);
    m_SyncData.length = length;
}

void DmxSerial::Update(const uint8_t* data, uint32_t length) {
    assert(data != nullptr);

    for (uint32_t index = 0; index < files_count_; index++) {
        assert(m_aFileIndex[index] > 1);
        const int32_t kOffset = m_aFileIndex[index] - 1;

        assert(kOffset < static_cast<int32_t>(sizeof(dmx_data_)));

        if (static_cast<uint32_t>(kOffset) >= length) {
            continue;
        }

        if (dmx_data_[kOffset] != data[kOffset]) {
            dmx_data_[kOffset] = data[kOffset];

            //			DEBUG_PRINTF("nPort=%d, index=%d, m_aFileIndex[index]=%d, nOffset=%d, dmx_data_[nOffset]=%d", nPort, index, m_aFileIndex[index],
            // nOffset, dmx_data_[nOffset]);

            if (m_pDmxSerialChannelData[index] != nullptr) {
                uint32_t channel_data_length;
                const uint8_t* serial_data = m_pDmxSerialChannelData[index]->GetData(dmx_data_[kOffset], channel_data_length);

                if (channel_data_length == 0) {
                    continue;
                }

                Serial::Send(serial_data, channel_data_length);
            }
        }
    }
}

void DmxSerial::Sync([[maybe_unused]] uint32_t port_index) {
    // No actions here
}

void DmxSerial::Sync() {
    Update(m_SyncData.data, m_SyncData.length);
}

void DmxSerial::Print() {
    Serial::Print();

    printf("Files : %u\n", static_cast<unsigned>(files_count_));
    puts("DMX");
    printf(" First channel : %u\n", static_cast<unsigned>(m_aFileIndex[0]));
    printf(" Last channel  : %u\n", static_cast<unsigned>(m_nDmxLastSlot));
}

void DmxSerial::ScanDirectory() {
    // We can only run this once, for now
    assert(m_pDmxSerialChannelData[0] == nullptr);

    DIR* dirp;
    struct dirent* dp;
    files_count_ = 0;

    if ((dirp = opendir(".")) == nullptr) {
        perror("couldn't open '.'");

        for (uint32_t i = 0; i < DmxSerialFile::MAX_NUMBER; i++) {
            m_aFileIndex[i] = -1;
        }

        return;
    }

    do {
        if ((dp = readdir(dirp)) != nullptr) {
            if (dp->d_type == DT_DIR) {
                continue;
            }

            int32_t file_number;
            if (!CheckFileName(dp->d_name, file_number)) {
                continue;
            }

            m_aFileIndex[files_count_] = file_number;

            DEBUG_PRINTF("[%d] found %s", files_count_, dp->d_name);

            files_count_++;

            if (files_count_ == DmxSerialFile::MAX_NUMBER) {
                break;
            }
        }
    } while (dp != nullptr);

    // Sort
    for (uint32_t i = 0; i < files_count_; i++) {
        for (uint32_t j = 0; j < files_count_; j++) {
            if (m_aFileIndex[j] > m_aFileIndex[i]) {
                auto tmp = m_aFileIndex[i];
                m_aFileIndex[i] = m_aFileIndex[j];
                m_aFileIndex[j] = tmp;
            }
        }
    }

    m_nDmxLastSlot = static_cast<uint16_t>(m_aFileIndex[files_count_ - 1]);

    for (uint32_t i = files_count_; i < DmxSerialFile::MAX_NUMBER; i++) {
        m_aFileIndex[i] = -1;
    }

    static_cast<void>(closedir(dirp));

#ifndef NDEBUG
    printf("%u\n", static_cast<unsigned>(files_count_));
#endif

    for (uint32_t index = 0; index < files_count_; index++) {
#ifndef NDEBUG
        printf("\tnIndex=%u -> %d\n", static_cast<unsigned>(index), static_cast<int>(m_aFileIndex[index]));
#endif

        m_pDmxSerialChannelData[index] = new DmxSerialChannelData;
        assert(m_pDmxSerialChannelData[index] != nullptr);

        char buffer[16];
        snprintf(buffer, sizeof(buffer) - 1, DMXSERIAL_FILE_PREFIX "%.3d" DMXSERIAL_FILE_SUFFIX, static_cast<int>(m_aFileIndex[index]));
        DEBUG_PUTS(buffer);
        m_pDmxSerialChannelData[index]->Parse(buffer);
    }

#ifndef NDEBUG
    for (uint32_t index = 0; index < files_count_; index++) {
        printf("\tindex=%u -> %d\n", static_cast<unsigned>(index), static_cast<int>(m_aFileIndex[index]));
        m_pDmxSerialChannelData[index]->Dump();
    }
#endif
}

void DmxSerial::EnableTFTP(bool enable_tftp) {
    DEBUG_ENTRY();

    if (enable_tftp == enable_tftp_) {
        DEBUG_EXIT();
        return;
    }

    DEBUG_PRINTF("bEnableTFTP=%d", enable_tftp);

    enable_tftp_ = enable_tftp;

    if (enable_tftp_) {
        assert(m_pDmxSerialTFTP == nullptr);
        m_pDmxSerialTFTP = new DmxSerialTFTP;
        assert(m_pDmxSerialTFTP != nullptr);
    } else {
        assert(m_pDmxSerialTFTP != nullptr);
        delete m_pDmxSerialTFTP;
        m_pDmxSerialTFTP = nullptr;
    }

    DEBUG_EXIT();
}

bool DmxSerial::DeleteFile(int32_t file_number) {
    DEBUG_PRINTF("file_number=%u", file_number);

    char file_name[DmxSerialFile::NAME_LENGTH + 1];

    if (FileNameCopyTo(file_name, sizeof(file_name), file_number)) {
        const int kResult = unlink(file_name);
        DEBUG_PRINTF("kResult=%d", kResult);
        DEBUG_EXIT();
        return (kResult == 0);
    }

    DEBUG_EXIT();
    return false;
}

bool DmxSerial::DeleteFile(const char* delete_file_number) {
    assert(delete_file_number != nullptr);

    DEBUG_PUTS(delete_file_number);

    if (strlen(delete_file_number) != 3) {
        return false;
    }

    auto file_number = (delete_file_number[0] - '0') * 100;
    file_number += (delete_file_number[1] - '0') * 10;
    file_number += (delete_file_number[2] - '0');

    if (file_number > static_cast<int32_t>(DmxSerialFile::MAX_NUMBER)) {
        return false;
    }

    return DeleteFile(file_number);
}

// Explicit template instantiations
template void DmxSerial::SetData<true>(uint32_t, const uint8_t*, uint32_t);
template void DmxSerial::SetData<false>(uint32_t, const uint8_t*, uint32_t);

namespace cmd {
inline constexpr char kRequestFiles[] = "?files#";
inline constexpr char kGetTftp[] = "?tftp#";
inline constexpr char kSetTftp[] = "!tftp#";
inline constexpr char kRequestReload[] = "?reload##";
inline constexpr char kRequestDelete[] = "!delete#";
} // namespace cmd

namespace length {
inline constexpr auto kRequestFiles = sizeof(cmd::kRequestFiles) - 1;
inline constexpr auto kGetTftp = sizeof(cmd::kGetTftp) - 1;
inline constexpr auto kSetTftp = sizeof(cmd::kSetTftp) - 1;
inline constexpr auto kRequestReload = sizeof(cmd::kRequestReload) - 1;
inline constexpr auto kRequestDelete = sizeof(cmd::kRequestDelete) - 1;
} // namespace length

void DmxSerial::Input(const uint8_t* p, uint32_t size, uint32_t from_ip, [[maybe_unused]] uint16_t from_port) {
    if (__builtin_expect((size < 6), 0)) {
        return;
    }

    auto* buffer = const_cast<uint8_t*>(p);

    if (buffer[size - 1] == '\n') {
        size--;
    }

#ifndef NDEBUG
    debug::Dump(buffer, size);
#endif

    if (memcmp(buffer, cmd::kRequestFiles, length::kRequestFiles) == 0) {
        for (uint32_t i = 0; i < files_count_; i++) {
            const auto kLength = snprintf(reinterpret_cast<char*>(buffer), network::udp::kDataSize - 1, DMXSERIAL_FILE_PREFIX "%.3d" DMXSERIAL_FILE_SUFFIX "\n", static_cast<int>(m_aFileIndex[i]));
            network::udp::Send(handle_, buffer, kLength, from_ip, UDP::PORT);
        }
        return;
    }

    if ((size >= length::kGetTftp) && (memcmp(buffer, cmd::kGetTftp, length::kGetTftp) == 0)) {
        if (size == length::kGetTftp) {
            const auto kLength = snprintf(reinterpret_cast<char*>(buffer), network::udp::kDataSize - 1, "tftp:%s\n", enable_tftp_ ? "On" : "Off");
            network::udp::Send(handle_, buffer, kLength, from_ip, UDP::PORT);
            return;
        }

        if (size == length::kGetTftp + 3) {
            if (memcmp(&buffer[length::kGetTftp], "bin", 3) == 0) {
                network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&enable_tftp_), sizeof(bool), from_ip, UDP::PORT);
                return;
            }
        }
    }

    if ((size == length::kSetTftp + 1) && (memcmp(buffer, cmd::kSetTftp, length::kSetTftp) == 0)) {
        EnableTFTP(buffer[length::kSetTftp] != '0');
        return;
    }

    if (memcmp(buffer, cmd::kRequestReload, length::kRequestReload) == 0) {
        board::Reboot();
        return;
    }

    if ((size == length::kRequestDelete + 3) && (memcmp(buffer, cmd::kRequestDelete, length::kRequestDelete) == 0)) {
        buffer[length::kRequestDelete + 3] = '\0';

        if (DmxSerial::Get()->DeleteFile(reinterpret_cast<char*>(&buffer[length::kRequestDelete]))) {
            network::udp::Send(handle_, reinterpret_cast<const uint8_t*>("Success\n"), 8, from_ip, UDP::PORT);
        } else {
            const auto* error = strerror(errno);
            const auto kLength = snprintf(reinterpret_cast<char*>(buffer), network::udp::kDataSize - 1, "%s\n", error);
            network::udp::Send(handle_, buffer, kLength, from_ip, UDP::PORT);
        }
        return;
    }
}
