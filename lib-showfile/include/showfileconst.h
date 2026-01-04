/**
 * @file showfileconst.h
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SHOWFILECONST_H_
#define SHOWFILECONST_H_

namespace showfile
{
enum class Status
{
    kIdle,
    kPlaying,
    kStopped,
    kEnded,
    kRecording,
    kUndefined
};

inline const char kStatus[static_cast<int>(showfile::Status::kUndefined)][12] = {"Idle", "Playing", "Stopped", "Ended", "Recording"};

enum class Mode
{
    kPlayer,
    kRecorder,
    kUndefined
};

inline const char kMode[static_cast<int>(showfile::Mode::kUndefined)][10] = {"Player", "Recorder"};
} // namespace showfile

#endif  // SHOWFILECONST_H_
