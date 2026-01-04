/**
 * @file dmxnode_outputtype.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
 

#ifndef DMXNODE_OUTPUTTYPE_H_
#define DMXNODE_OUTPUTTYPE_H_

 #include <cstdint>

namespace dmxnode
{
enum class OutputType
{
    kDmx,
    kDmxRdm,
    kPixel,
    kPixelDmx,
    kPwm,
    kRgbPanel,
    kSerial,
    kOsc,
    kMonitor,
    kStepper,
    kPlayer,
    kArtNet,
    kTimeCode,
    kNone,
    kUndefined
};

inline constexpr const char* kOutputTypeNames[static_cast<uint32_t>(OutputType::kUndefined)] = 
{
	"DMX", 
	"DMX/RDM", 
	"Pixel",
	"Pixel/DMX", 
	"PWM", 
	"RGB Panel", 
	"Serial", 
	"OSC", 
	"Monitor", 
	"Stepper", 
	"Player", 
	"Art-Net", 
	"Timecode",
	"None"
};

inline const char* GetOutputType(OutputType type)
{
    if (type < OutputType::kUndefined)
    {
        return kOutputTypeNames[static_cast<uint32_t>(type)];
    }

    return "Undefined";
}
}  // namespace dmxnode

#if defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)
#define DMXNODE_OUTPUT_DMX
#endif

#if defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI)
#define DMXNODE_OUTPUT_PIXEL
#endif

#if defined(DMXNODE_OUTPUT_DMX) && defined(DMXNODE_OUTPUT_PIXEL)
#define DMXNODE_OUTPUT_PIXEL_DMX
#endif

#if defined(OUTPUT_DMX_STEPPER) && defined(OUTPUT_DMX_TLC59711)
#define DMXNODE_OUTPUT_STEPPER_TLC59711
#endif

#if defined(OUTPUT_DMX_PIXEL) && defined(RDM_RESPONDER) && !defined(NODE_ARTNET)
#define DMXNODE_OUTPUT_RDM_PIXEL
#endif

#if defined(OUTPUT_DMX_MONITOR)
#include "dmxmonitor.h"
#endif

#if defined(OUTPUT_DMX_ARTNET)
#include "artnetoutput.h"
#endif

#if defined(DMXNODE_OUTPUT_DMX)
#include "dmxsend.h"
#endif

#if defined(OUTPUT_DMX_PIXEL)
#include "pixeldmx.h"
using DmxPixelOutputType = PixelDmx;
#endif

#if defined(OUTPUT_DMX_PIXEL_MULTI)
#include "pixeldmxmulti.h"
using DmxPixelOutputType = PixelDmxMulti;
#endif

#if defined(OUTPUT_DMX_PCA9685)
#define DMXNODE_OUTPUT_PCA9685
#include "pca9685dmxled.h"
#include "pca9685dmxservo.h"
#endif

#if defined(OUTPUT_DMX_SERIAL)
#define DMXNODE_OUTPUT_SERIAL
#include "dmxserial.h"
#endif

#if defined(OUTPUT_DMX_STEPPER)
#include "sparkfundmx.h"
#endif

#if defined(OUTPUT_DMX_TLC59711)
#include "tlc59711dmx.h"
#endif

#if defined(OUTPUT_DMX_NULL)
#include "dmxnodeoutputtypenull.h"
#endif

#if defined(DMXNODE_OUTPUT_PIXEL_DMX)
#include "dmxnodewith4.h"
using DmxNodeOutputType = DmxNodeWith4<CONFIG_DMXNODE_DMX_PORT_OFFSET>;
#elif defined(DMXNODE_OUTPUT_DMX)
using DmxNodeOutputType = DmxSend;
#elif defined(DMXNODE_OUTPUT_RDM_PIXEL)
#include "dmxnodeoutputrdmpixel.h"
using DmxNodeOutputType = DmxNodeOutputRdmPixel;
#elif defined(OUTPUT_DMX_ARTNET)
using DmxNodeOutputType = ArtNetOutput;
#elif defined(OUTPUT_DMX_MONITOR)
using DmxNodeOutputType = DmxMonitor;
#elif defined(OUTPUT_DMX_PIXEL)
using DmxNodeOutputType = PixelDmx;
#elif defined(OUTPUT_DMX_PIXEL_MULTI)
using DmxNodeOutputType = PixelDmxMulti;
#elif defined(OUTPUT_DMX_PCA9685_LED)
using DmxNodeOutputType = PCA9685DmxLed;
#elif defined(OUTPUT_DMX_PCA9685_SERVO)
using DmxNodeOutputType = PCA9685DmxServo;
#elif defined(DMXNODE_OUTPUT_STEPPER_TLC59711)
#include "dmxnodechain.h"
using DmxNodeOutputType = DmxNodeChain;
#elif defined(OUTPUT_DMX_SERIAL)
using DmxNodeOutputType = DmxSerial;
#elif defined(OUTPUT_DMX_STEPPER)
using DmxNodeOutputType = SparkFunDmx;
#elif defined(OUTPUT_DMX_PCA9685)
using DmxNodeOutputType = PCA9685DmxSet;
#elif defined(OUTPUT_DMX_TLC59711)
using DmxNodeOutputType = TLC59711Dmx;
#elif defined(OUTPUT_DMX_NULL)
using DmxNodeOutputType = DmxNodeOutputTypeNull;
#endif

namespace dmxnode
{
#if defined(DMXNODE_OUTPUT_DMX) && defined(RDM_CONTROLLER)
inline constexpr auto kOutputType = OutputType::kDmxRdm;
#elif defined(DMXNODE_OUTPUT_DMX)
inline constexpr auto kOutputType = OutputType::kDmx;
#elif defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI)
inline constexpr auto kOutputType = OutputType::kPixel;
#elif defined(DMXNODE_OUTPUT_PIXEL_DMX)
inline constexpr auto kOutputType = OutputType::kPixelDmx;
#elif defined(OUTPUT_DMX_NULL)
inline constexpr auto kOutputType = OutputType::kNone;
#else
inline constexpr auto kOutputType = OutputType::kUndefined;
#endif
} // namespace dmxnode

#endif  // DMXNODE_OUTPUTTYPE_H_
