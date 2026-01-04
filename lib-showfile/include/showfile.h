/**
 * @file showfile.h
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

#ifndef SHOWFILE_H_
#define SHOWFILE_H_

#include <cstdio>

#include "showfileconst.h"
#include "showfiletftp.h"
#include "showfileformat.h"
#include "showfileprotocol.h"

#if defined(CONFIG_SHOWFILE_ENABLE_OSC)
#include "showfileosc.h"
#endif

#if defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI)
#define CONFIG_SHOWFILE_ENABLE_MASTER
#endif

 #include "firmware/debug/debug_debug.h"

namespace showfile
{
bool FilenameCopyto(char* show_file_name, uint32_t length, int32_t show_file_number);
bool FilenameCheck(const char* show_file_name, int32_t& show_file_number);
} // namespace showfile

class ShowFile final : public ShowFileFormat
{
   public:
#if defined(CONFIG_SHOWFILE_ENABLE_OSC)
    explicit ShowFile(uint16_t port_incoming = osc::port::DEFAULT_INCOMING, uint16_t port_outgoing = osc::port::DEFAULT_OUTGOING);
#else
    ShowFile();
#endif

    ShowFile(const ShowFile&) = delete;
    ShowFile& operator=(const ShowFile&) = delete;

    void Play()
    {
        DEBUG_ENTRY();

        if ((mode_ == showfile::Mode::kRecorder) || (status_ == showfile::Status::kPlaying) || (status_ == showfile::Status::kRecording))
        {
            DEBUG_EXIT();
            return;
        }

        EnableTFTP(false);

        if (m_pShowFile != nullptr)
        {
            ShowFileFormat::ShowFileStart();
            SetStatus(showfile::Status::kPlaying);
        }
        else
        {
            SetStatus(showfile::Status::kIdle);
        }

        DEBUG_EXIT();
    }

    void Stop()
    {
        DEBUG_ENTRY();

        if (m_pShowFile != nullptr)
        {
            ShowFileFormat::ShowFileStop();

            if ((status_ == showfile::Status::kStopped) || (status_ == showfile::Status::kRecording))
            {
                if (status_ == showfile::Status::kRecording)
                {
                    fclose(m_pShowFile);
                }
                SetStatus(showfile::Status::kIdle);
            }
            else
            {
                SetStatus(showfile::Status::kStopped);
            }
        }

        DEBUG_EXIT();
    }

    void Resume()
    {
        DEBUG_ENTRY();

        if (status_ != showfile::Status::kStopped)
        {
            DEBUG_EXIT();
            return;
        }

        if (m_pShowFile != nullptr)
        {
            ShowFileFormat::ShowFileResume();
            SetStatus(showfile::Status::kPlaying);
        }

        DEBUG_EXIT();
    }

#if !defined(CONFIG_SHOWFILE_DISABLE_RECORD)
    void Record()
    {
        DEBUG_ENTRY();

        if ((mode_ == showfile::Mode::kPlayer) || (status_ != showfile::Status::kIdle))
        {
            DEBUG_EXIT();
            return;
        }

        if (m_pShowFile != nullptr)
        {
            ShowFileFormat::ShowFileRecord();
            SetStatus(showfile::Status::kRecording);
        }
        else
        {
            SetStatus(showfile::Status::kIdle);
        }

        DEBUG_EXIT();
    }
#endif

    void Run() { ShowFileFormat::ShowFileRun(status_ == showfile::Status::kPlaying); }

    void Print()
    {
        puts("Showfile");
        if (showfile_name_current_[0] != '\0')
        {
            printf(" %s\n", showfile_name_current_);
        }
        if (auto_play_)
        {
            puts(" Auto play");
        }
        printf(" %s\n", m_bDoLoop ? "Looping" : "Not looping");
#if defined(CONFIG_SHOWFILE_DISABLE_RECORD)
        puts(" Recorder is disabled.");
#endif
        ShowFileFormat::ShowFilePrint();
#if defined(CONFIG_SHOWFILE_ENABLE_OSC)
        showfile_osc_.Print();
#endif
    }

    showfile::Mode GetMode() const { return mode_; }

    void SetStatus(showfile::Status status);

    showfile::Status GetStatus() const { return status_; }

    void LoadShows();

    void UnloadShows()
    {
        shows_ = 0;

        for (auto& file_index : show_file_number_)
        {
            file_index = -1;
        }

        show_file_current_ = showfile::kFileMaxNumber + 1U;
    }

    void SetPlayerShowFileCurrent(int32_t show_file_number);

#if !defined(CONFIG_SHOWFILE_DISABLE_RECORD)
    void SetRecorderShowFileCurrent(int32_t show_file_number);
#endif

    const char* GetShowFileNameCurrent() const { return static_cast<const char*>(showfile_name_current_); }

    int32_t GetShowFileCurrent() const { return show_file_current_; }

    uint32_t GetShows()
    {
        if (shows_ == 0)
        {
            LoadShows();
        }
        return shows_;
    }

    int32_t GetPlayerShowFile(uint32_t index) const
    {
        if (index < sizeof(show_file_number_) / sizeof(show_file_number_[0]))
        {
            return show_file_number_[index];
        }

        return -1;
    }

    bool DeleteShowFile(int32_t show_file_number);

    bool GetShowFileSize(int32_t show_file_number, uint32_t& size);

    void DoLoop(bool do_loop) { m_bDoLoop = do_loop; }

    bool GetDoLoop() const { return m_bDoLoop; }

    void SetAutoStart(bool auto_play) { auto_play_ = auto_play; }

    bool IsAutoStart() const { return auto_play_; }

    bool IsSyncDisabled() { return ShowFileFormat::IsSyncDisabled(); }

    void BlackOut()
    {
#if defined(CONFIG_SHOWFILE_ENABLE_MASTER)
        Stop();
        ShowFileFormat::BlackOut();
#endif
    }

    void SetMaster([[maybe_unused]] uint32_t master)
    {
#if defined(CONFIG_SHOWFILE_ENABLE_MASTER)
        ShowFileFormat::SetMaster(master);
#endif
    }

    /*
     * TFTP
     */

    void EnableTFTP(bool enable_tftp);

    bool IsTFTPEnabled() const
    {
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
        return enable_tftp_;
#else
        return false;
#endif
    }

    /*
     * OSC
     */

#if defined(CONFIG_SHOWFILE_ENABLE_OSC)
    void SetOscPortIncoming(uint16_t port_incoming) { showfile_osc_.SetPortIncoming(port_incoming); }
    uint16_t GetOscPortIncoming() const { return showfile_osc_.GetPortIncoming(); }

    void SetOscPortOutgoing(uint16_t port_outgoing) { showfile_osc_.SetPortOutgoing(port_outgoing); }

    uint16_t GetOscPortOutgoing() const { return showfile_osc_.GetPortOutgoing(); }
#endif

    static ShowFile& Instance()
    {
        assert(s_this != nullptr); // Ensure that s_this is valid
        return *s_this;
    }

   private:
    void OpenFile(showfile::Mode mode, int32_t show_file_number);
    bool AddShow(int32_t show_file_number);

   private:
#if defined(CONFIG_SHOWFILE_ENABLE_OSC)
    ShowFileOSC showfile_osc_;
#endif
    showfile::Mode mode_{showfile::Mode::kPlayer};
    showfile::Status status_{showfile::Status::kIdle};
    char showfile_name_current_[showfile::kFileNameLength + 1]; // Including '\0'
    uint32_t shows_{0};
    int32_t show_file_number_[showfile::kFileMaxNumber + 1];
    bool auto_play_{false};
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
    bool enable_tftp_{false};
    ShowFileTFTP* showfile_tftp_{nullptr};
#endif
    static inline ShowFile* s_this;
};

#endif  // SHOWFILE_H_
