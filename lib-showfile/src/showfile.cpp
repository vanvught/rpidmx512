/**
 * @file showfile.cpp
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

#include <cstdint>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>
#include <cassert>

#include "showfile.h"
#include "showfiletftp.h"
#include "showfiledisplay.h"
#include "json/showfileparams.h"

#if defined CONFIG_USB_HOST_MSC
#include "device/usb/host.h"
#endif

#include "hal_statusled.h"

 #include "firmware/debug/debug_debug.h"

#if defined(CONFIG_SHOWFILE_ENABLE_OSC)
ShowFile::ShowFile(uint16_t port_incoming, uint16_t port_outgoing) : showfile_osc_(port_incoming, port_outgoing)
{
#else
ShowFile::ShowFile()
{
#endif
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    showfile_name_current_[0] = '\0';

    json::ShowFileParams showfile_params;
    showfile_params.Load();
    showfile_params.Set();

    DEBUG_EXIT();
}

void ShowFile::OpenFile(showfile::Mode mode, int32_t show_file_number)
{
    DEBUG_ENTRY();

    if (showfile::FilenameCopyto(showfile_name_current_, sizeof(showfile_name_current_), show_file_number))
    {
#if defined CONFIG_USB_HOST_MSC
        if (usb::host::get_status() != usb::host::Status::READY)
        {
            DEBUG_EXIT();
            return;
        }
#endif

        if (m_pShowFile != nullptr)
        {
            if (fclose(m_pShowFile) != 0)
            {
                perror("fclose()");
            }
            m_pShowFile = nullptr;
        }

        m_pShowFile = fopen(showfile_name_current_, mode == showfile::Mode::kRecorder ? "w" : "r");

        if (m_pShowFile == nullptr)
        {
            perror(const_cast<char*>(showfile_name_current_));
            showfile_name_current_[0] = '\0';
        }
        else
        {
            show_file_current_ = show_file_number;
            mode_ = mode;
        }

        showfile::DisplayFilename(showfile_name_current_, show_file_current_);
        showfile::DisplayStatus();

        DEBUG_EXIT();
        return;
    }

    DEBUG_EXIT();
    return;
}

void ShowFile::SetPlayerShowFileCurrent(int32_t show_file_number)
{
    DEBUG_ENTRY();
    if (status_ != showfile::Status::kIdle)
    {
        DEBUG_EXIT();
        return;
    }

    DEBUG_PRINTF("show_file_number=%d", show_file_number);

    OpenFile(showfile::Mode::kPlayer, show_file_number);

    DEBUG_EXIT();
}

#if !defined(CONFIG_SHOWFILE_DISABLE_RECORD)
void ShowFile::SetRecorderShowFileCurrent(int32_t show_file_number)
{
    DEBUG_ENTRY();

    if (status_ != showfile::Status::kIdle)
    {
        DEBUG_EXIT();
        return;
    }

    DEBUG_PRINTF("show_file_number=%d", show_file_number);

    if (AddShow(show_file_number))
    {
        OpenFile(showfile::Mode::kRecorder, show_file_number);

        DEBUG_EXIT();
        return;
    }

    DEBUG_EXIT();
}
#endif

bool ShowFile::DeleteShowFile(int32_t show_file_number)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("show_file_number=%d", show_file_number);

    char file_name[showfile::kFileNameLength + 1U];

    if (showfile::FilenameCopyto(file_name, sizeof(file_name), show_file_number))
    {
        if (show_file_number == show_file_current_)
        {
            if (fclose(m_pShowFile) != 0)
            {
                perror("fclose()");
            }
            m_pShowFile = nullptr;
            show_file_current_ = -1;
        }
        const auto kResult = unlink(file_name);
        DEBUG_PRINTF("kResult=%d", kResult);
        DEBUG_EXIT();
        return (kResult == 0);
    }

    DEBUG_EXIT();
    return false;
}

bool ShowFile::GetShowFileSize(int32_t show_file_number, uint32_t& size)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("show_file_number=%d", show_file_number);

    char file_name[showfile::kFileNameLength + 1U];

    if (showfile::FilenameCopyto(file_name, sizeof(file_name), show_file_number))
    {
        auto* file = fopen(file_name, "r");
        if (file != nullptr)
        {
            if (fseek(file, 0L, SEEK_END) == 0)
            {
                size = static_cast<uint32_t>(ftell(file));
                fclose(file);
                DEBUG_PRINTF("size=%u", size);
                DEBUG_EXIT();
                return true;
            }
        }
        else
        {
            perror("fopen()");
        }
    }

    DEBUG_EXIT();
    return false;
}

bool ShowFile::AddShow(int32_t show_file_number)
{
    DEBUG_ENTRY();

    if (shows_ == (showfile::kFileMaxNumber))
    {
        DEBUG_EXIT();
        return false;
    }

    if (shows_ == 0)
    {
        show_file_number_[0] = static_cast<int8_t>(show_file_number);
    }
    else
    {
        for (auto& show : show_file_number_)
        {
            if (show == show_file_number)
            {
                DEBUG_EXIT();
                return true;
            }
        }

        int32_t i = static_cast<int32_t>(shows_) - 1;
        while ((show_file_number < show_file_number_[i]) && i >= 0)
        {
            show_file_number_[i + 1] = show_file_number_[i];
            i--;
        }

        show_file_number_[i + 1] = static_cast<int8_t>(show_file_number);
    }

    shows_++;

    DEBUG_EXIT();
    return true;
}

void ShowFile::LoadShows()
{
    DEBUG_ENTRY();

#if defined(CONFIG_USB_HOST_MSC)
    auto* dirp = opendir("0:/");
#else
    auto* dirp = opendir(".");
#endif

    if (dirp == nullptr)
    {
        perror("opendir");
        DEBUG_EXIT();
        return;
    }

    for (auto& file_index : show_file_number_)
    {
        file_index = -1;
    }

    shows_ = 0;

    struct dirent* dp;

    do
    {
        if ((dp = readdir(dirp)) != nullptr)
        {
            if (dp->d_type == DT_DIR)
            {
                continue;
            }

            int32_t show_file_number;
            if (!showfile::FilenameCheck(dp->d_name, show_file_number))
            {
                continue;
            }

            DEBUG_PRINTF("Found %s", dp->d_name);

            if (!AddShow(show_file_number))
            {
                break;
            }
        }
    } while (dp != nullptr);

    closedir(dirp);

    DEBUG_EXIT();
}

void ShowFile::EnableTFTP([[maybe_unused]] bool enable_tftp)
{
    DEBUG_ENTRY();

#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
    if (enable_tftp == enable_tftp_)
    {
        DEBUG_EXIT();
        return;
    }

    enable_tftp_ = enable_tftp;

    if (enable_tftp_)
    {
        assert(showfile_tftp_ == nullptr);

        Stop();

        if (m_pShowFile != nullptr)
        {
            if (fclose(m_pShowFile) != 0)
            {
                perror("fclose(m_pShowFile)");
            }
            m_pShowFile = nullptr;
        }

        showfile_tftp_ = new ShowFileTFTP;
        assert(showfile_tftp_ != nullptr);
    }
    else
    {
        assert(showfile_tftp_ != nullptr);

        delete showfile_tftp_;
        showfile_tftp_ = nullptr;

        LoadShows();
        SetPlayerShowFileCurrent(show_file_current_);
        SetStatus(showfile::Status::kIdle);
    }

    showfile::DisplayStatus();
#endif

    DEBUG_EXIT();
}

void ShowFile::SetStatus(showfile::Status status)
{
    DEBUG_ENTRY();

    if (status == status_)
    {
        DEBUG_EXIT();
        return;
    }

    status_ = status;

    switch (status_)
    {
        case showfile::Status::kIdle:
            ShowFileFormat::DoRunCleanupProcess(true);
            hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
            break;
        case showfile::Status::kPlaying:
        case showfile::Status::kRecording:
            ShowFileFormat::DoRunCleanupProcess(false);
            hal::statusled::SetMode(hal::statusled::Mode::DATA);
            break;
        case showfile::Status::kStopped:
        case showfile::Status::kEnded:
            ShowFileFormat::DoRunCleanupProcess(true);
            hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
            break;
        default:
            break;
    }

    showfile::DisplayStatus();

    DEBUG_EXIT();
}
