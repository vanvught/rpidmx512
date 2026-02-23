/**
 * @file showfileosc.cpp
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

#if defined(DEBUG_SHOWFILEOSC)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <cassert>

#include "showfileosc.h"
#include "showfile.h"

#include "oscsimplemessage.h"
#include "oscsimplesend.h"

 #include "firmware/debug/debug_debug.h"

namespace cmd
{
static constexpr char kStart[] = "start";
static constexpr char kStop[] = "stop";
static constexpr char kResume[] = "resume";
static constexpr char kShow[] = "show";
static constexpr char kLoop[] = "loop";
static constexpr char kBlackout[] = "blackout";
#if defined(CONFIG_SHOWFILE_ENABLE_MASTER)
static constexpr char kMaster[] = "master";
#endif
static constexpr char kTftp[] = "tftp";
static constexpr char kDelete[] = "delete";
// TouchOSC specific
static constexpr char kReload[] = "reload";
static constexpr char kIndex[] = "index";
} // namespace cmd

namespace length
{
static constexpr uint32_t kStart = sizeof(cmd::kStart) - 1;
static constexpr uint32_t kStop = sizeof(cmd::kStop) - 1;
static constexpr uint32_t kResume = sizeof(cmd::kResume) - 1;
static constexpr uint32_t kShow = sizeof(cmd::kShow) - 1;
static constexpr uint32_t kLoop = sizeof(cmd::kLoop) - 1;
static constexpr uint32_t kBo = sizeof(cmd::kBlackout) - 1;
#if defined(CONFIG_SHOWFILE_ENABLE_MASTER)
static constexpr uint32_t kMaster = sizeof(cmd::kMaster) - 1;
#endif
static constexpr uint32_t kTftp = sizeof(cmd::kTftp) - 1;
static constexpr uint32_t kDelete = sizeof(cmd::kDelete) - 1;
// TouchOSC specific
static constexpr uint32_t kReload = sizeof(cmd::kReload) - 1;
static constexpr uint32_t kIndex = sizeof(cmd::kIndex) - 1;
} // namespace length

void ShowFileOSC::Process()
{
    DEBUG_PRINTF("[%s] %d,%d %s", buffer_, static_cast<int>(strlen(reinterpret_cast<const char*>(buffer_))), static_cast<int>(showfileosc::kPathLength),
                 &buffer_[showfileosc::kPathLength]);

    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kStart, length::kStart) == 0)
    {
        ShowFile::Instance().Play();
        SendStatus();
        DEBUG_PUTS("ActionStart");
        return;
    }

    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kStop, length::kStop) == 0)
    {
        ShowFile::Instance().Stop();
        SendStatus();
        DEBUG_PUTS("ActionStop");
        return;
    }

    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kResume, length::kResume) == 0)
    {
        ShowFile::Instance().Resume();
        SendStatus();
        DEBUG_PUTS("ActionResume");
        return;
    }

    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kShow, length::kShow) == 0)
    {
        OscSimpleMessage msg(buffer_, bytes_received_);

        const auto kValue = msg.GetInt(0);

        if (kValue <= showfile::kFileMaxNumber)
        {
            ShowFile::Instance().SetPlayerShowFileCurrent(kValue);
            SendStatus();
        }

        DEBUG_PRINTF("Show %d", kValue);
        return;
    }

    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kLoop, length::kLoop) == 0)
    {
        OscSimpleMessage msg(buffer_, bytes_received_);

        int value;

        if (msg.GetType(0) == osc::type::INT32)
        {
            value = msg.GetInt(0);
        }
        else if (msg.GetType(0) == osc::type::FLOAT)
        { // TouchOSC
            value = static_cast<int>(msg.GetFloat(0));
        }
        else
        {
            return;
        }

        ShowFile::Instance().DoLoop(value != 0);
        SendStatus();
        showfile::DisplayStatus();

        DEBUG_PRINTF("Loop %d", value != 0);
        return;
    }

    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kBlackout, length::kBo) == 0)
    {
        ShowFile::Instance().BlackOut();
        SendStatus();
        DEBUG_PUTS("Blackout");
        return;
    }

#if defined(CONFIG_SHOWFILE_ENABLE_MASTER)
    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kMaster, length::kMaster) == 0)
    {
        OscSimpleMessage msg(buffer_, bytes_received_);

        int value;

        if (msg.GetType(0) == osc::type::INT32)
        {
            value = msg.GetInt(0);
        }
        else if (msg.GetType(0) == osc::type::FLOAT)
        { // TouchOSC
            value = static_cast<int>(msg.GetFloat(0));
        }
        else
        {
            return;
        }

        if ((value >= 0) && (value <= 255))
        {
            ShowFile::Instance().SetMaster(static_cast<uint32_t>(value));
        }

        DEBUG_PRINTF("Master %d", value);
        return;
    }
#endif

    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kTftp, length::kTftp) == 0)
    {
        OscSimpleMessage msg(buffer_, bytes_received_);

        int value;

        if (msg.GetType(0) == osc::type::INT32)
        {
            value = msg.GetInt(0);
        }
        else if (msg.GetType(0) == osc::type::FLOAT)
        { // TouchOSC
            value = static_cast<int>(msg.GetFloat(0));
        }
        else
        {
            return;
        }

        ShowFile::Instance().EnableTFTP(value != 0);
        SendStatus();

        DEBUG_PRINTF("TFTP %d", value != 0);
        return;
    }

    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kDelete, length::kDelete) == 0)
    {
        OscSimpleMessage msg(buffer_, bytes_received_);

        int32_t value = 255;

        if (msg.GetType(0) == osc::type::INT32)
        {
            value = msg.GetInt(0);
        }
        else
        {
            return;
        }

        DEBUG_PRINTF("value=%d", value);

        if (value <= showfile::kFileMaxNumber)
        {
            char show_file_name[showfile::kFileNameLength + 1];

            showfile::FilenameCopyto(show_file_name, sizeof(show_file_name), value);

            OscSimpleSend msg_name(handle_, remote_ip_, port_outgoing_, "/showfile/name", "s", show_file_name);

            if (ShowFile::Instance().DeleteShowFile(value))
            {
                OscSimpleSend msg_status(handle_, remote_ip_, port_outgoing_, "/showfile/status", "s", "Deleted");
            }
            else
            {
                OscSimpleSend msg_status(handle_, remote_ip_, port_outgoing_, "/showfile/status", "s", "Not deleted");
            }
        }

        DEBUG_PRINTF("Delete %d", value);
        return;
    }

    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kIndex, length::kIndex) == 0)
    {
        OscSimpleMessage msg(buffer_, bytes_received_);

        if (msg.GetType(0) != osc::type::FLOAT)
        {
            return;
        }

        const auto kIndex = static_cast<uint32_t>(msg.GetFloat(0));

        DEBUG_PRINTF("kIndex=%u", kIndex);

        const auto kShow = ShowFile::Instance().GetPlayerShowFile(kIndex);

        if (kShow < 0)
        {
            return;
        }

        DEBUG_PRINTF("kShow=%d", kShow);

        ShowFile::Instance().SetPlayerShowFileCurrent(kShow);
        SendStatus();

        return;
    }

    // TouchOSC
    if (memcmp(&buffer_[showfileosc::kPathLength], cmd::kReload, length::kReload) == 0)
    {
        ShowFiles();
        DEBUG_PUTS("Reload");
        return;
    }
}

// TouchOSC
void ShowFileOSC::SendStatus()
{
    OscSimpleSend msg_name(handle_, remote_ip_, port_outgoing_, "/showfile/name", "s", ShowFile::Instance().GetShowFileNameCurrent());

    const auto kStatus = ShowFile::Instance().GetStatus();
    assert(kStatus != showfile::Status::kUndefined);

    OscSimpleSend msg_status(handle_, remote_ip_, port_outgoing_, "/showfile/status", "s", showfile::kStatus[static_cast<int>(kStatus)]);
}

// TouchOSC
void ShowFileOSC::ShowFiles()
{
    char path[64];
    char value[4];

    uint32_t i;

    for (i = 0; i < ShowFile::Instance().GetShows(); i++)
    {
        snprintf(path, sizeof(path) - 1, "/showfile/%u/show", static_cast<unsigned int>(i));
        snprintf(value, sizeof(value) - 1, "%.2u", static_cast<unsigned int>(ShowFile::Instance().GetPlayerShowFile(i)));
        OscSimpleSend msg_status(handle_, remote_ip_, port_outgoing_, path, "s", value);
    }

    for (; i < showfileosc::kMaxFilesEntries; i++)
    {
        snprintf(path, sizeof(path) - 1, "/showfile/%u/show", static_cast<unsigned int>(i));
        OscSimpleSend msg_status(handle_, remote_ip_, port_outgoing_, path, "s", "  ");
    }

    SendStatus();
}
