/**
 * @file kjsoninfos.cpp
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

#include <cstddef>
#include <cstdint>

#include "common/utils/utils_hash.h"
#include "http/json_infos.h"

#include "dmxnode_nodetype.h"
#include "dmxnode_outputtype.h"

namespace json
{
uint32_t GetList(char*, uint32_t);
uint32_t GetVersion(char*, uint32_t);
uint32_t GetUptime(char*, uint32_t);

uint32_t GetTimeofday(char*, uint32_t);
void SetTimeofday(const char*, uint32_t);
uint32_t ShowFileDirectory(char*, uint32_t);

namespace status
{
uint32_t Identify(char*, uint32_t);
uint32_t Display(char*, uint32_t);
uint32_t Dmx(char*, uint32_t);
uint32_t Rdm(char*, uint32_t);
uint32_t RdmQueue(char*, uint32_t);
uint32_t ShowFile(char*, uint32_t);
uint32_t Pixel(char*, uint32_t);
uint32_t PixelDmx(char*, uint32_t);

namespace net
{
uint32_t Phy(char*, uint32_t);
uint32_t Emac(char*, uint32_t);
}
} // namespace status

namespace storage
{
uint32_t GetDirectory(char*, uint32_t);
}

namespace action
{
void Set(const char*, uint32_t);
void SetShowFile(const char*, uint32_t);
}

namespace config
{
uint32_t GetDirectory(char*, uint32_t);

uint32_t GetRemoteConfig(char*, uint32_t);
void SetRemoteConfig(const char*, uint32_t);

uint32_t GetGlobal(char*, uint32_t);
void SetGlobal(const char*, uint32_t);

uint32_t GetNetwork(char*, uint32_t);
void SetNetwork(const char*, uint32_t);

uint32_t GetDisplayUdf(char*, uint32_t);
void SetDisplayUdf(const char*, uint32_t);

uint32_t GetDmxNode(char*, uint32_t);
void SetDmxNode(const char*, uint32_t);

uint32_t GetArtNet(char*, uint32_t);
void SetArtNet(const char*, uint32_t);

uint32_t GetE131(char*, uint32_t);
void SetE131(const char*, uint32_t);

uint32_t GetOscClient(char*, uint32_t);
void SetOscClient(const char*, uint32_t);

uint32_t GetOscServer(char*, uint32_t);
void SetOscServer(const char*, uint32_t);

// RDM Device
uint32_t GetRdmDevice(char*, uint32_t);
void SetRdmDevice(const char*, uint32_t);
// RDM Sensors
uint32_t GetRdmSensors(char*, uint32_t);
void SetRdmSensors(const char*, uint32_t);

uint32_t GetDmxSend(char*, uint32_t);
void SetDmxSend(const char*, uint32_t);

uint32_t GetPixelDmx(char*, uint32_t);
void SetPixelDmx(const char*, uint32_t);

uint32_t GetPca9685Dmx(char*, uint32_t);
void SetPca9685Dmx(const char*, uint32_t);

uint32_t GetTlc59711Dmx(char*, uint32_t);
void SetTlc59711Dmx(const char*, uint32_t);

uint32_t GetDmxMonitor(char*, uint32_t);
void SetDmxMonitor(const char*, uint32_t);

uint32_t GetRgbPanel(char*, uint32_t);
void SetRgbPanel(const char*, uint32_t);

uint32_t GetDmxSerial(char*, uint32_t);
void SetDmxSerial(const char*, uint32_t);

uint32_t GetShowFile(char*, uint32_t);
void SetShowFile(const char*, uint32_t);

uint32_t GetGps(char*, uint32_t);
void SetGps(const char*, uint32_t);

uint32_t GetTcNet(char*, uint32_t);
void SetTcNet(const char*, uint32_t);

uint32_t GetLtc(char*, uint32_t);
void SetLtc(const char*, uint32_t);

uint32_t GetLtcDisplay(char*, uint32_t);
void SetLtcDisplay(const char*, uint32_t);

uint32_t GetLtcEtc(char*, uint32_t);
void SetLtcEtc(const char*, uint32_t);

// Stepper
uint32_t GetSparkFunDmx(char*, uint32_t);
void SetSparkFunDmx(const char*, uint32_t);
uint32_t GetDmxL6470Motor0(char*, uint32_t);
void SetDmxL6470Motor0(const char*, uint32_t);

uint32_t GetDmxL6470Motor1(char*, uint32_t);
void SetDmxL6470Motor1(const char*, uint32_t);

uint32_t GetDmxL6470Motor2(char*, uint32_t);
void SetDmxL6470Motor2(const char*, uint32_t);

uint32_t GetDmxL6470Motor3(char*, uint32_t);
void SetDmxL6470Motor3(const char*, uint32_t);

uint32_t GetDmxL6470Motor4(char*, uint32_t);
void SetDmxL6470Motor4(const char*, uint32_t);

uint32_t GetDmxL6470Motor5(char*, uint32_t);
void SetDmxL6470Motor5(const char*, uint32_t);

uint32_t GetDmxL6470Motor6(char*, uint32_t);
void SetDmxL6470Motor6(const char*, uint32_t);

uint32_t GetDmxL6470Motor7(char*, uint32_t);
void SetDmxL6470Motor7(const char*, uint32_t);
} // namespace config

#define ENTRY(get, set, del, filename_literal, label) MakeJsonFileInfo(get, set, del, filename_literal, Fnv1a32(filename_literal, static_cast<uint8_t>(sizeof(filename_literal) - 1)), label)

constexpr Info kFileInfos[] = {
	ENTRY(GetList, nullptr, nullptr, "list", nullptr),
	ENTRY(GetVersion, nullptr, nullptr, "version", nullptr),
	ENTRY(GetUptime, nullptr, nullptr, "uptime", nullptr),
    ENTRY(GetTimeofday, SetTimeofday, nullptr, "timedate", nullptr),
    // Status
    ENTRY(status::Identify, nullptr, nullptr, "status/identify", nullptr),
    ENTRY(status::Display, nullptr, nullptr, "status/display", nullptr),
    ENTRY(status::net::Phy, nullptr, nullptr, "status/phy", nullptr),
    ENTRY(status::net::Emac, nullptr, nullptr, "status/emac", nullptr),
#if defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)  
    ENTRY(status::Dmx, nullptr, nullptr, "status/dmx", nullptr),
#endif    
#if defined (DMXNODE_OUTPUT_PIXEL)
	ENTRY(status::Pixel, nullptr, nullptr, "status/pixel", nullptr),
	ENTRY(status::PixelDmx, nullptr, nullptr, "status/pixeldmx", nullptr),
#endif
#if defined (RDM_CONTROLLER)    
    ENTRY(status::Rdm, nullptr, nullptr, "status/rdm", nullptr),    
    ENTRY(status::RdmQueue, nullptr, nullptr, "status/rdm/queue", nullptr),
#endif
#if defined (NODE_SHOWFILE)  
	ENTRY(status::ShowFile, nullptr, nullptr, "status/showfile", nullptr),
#endif      
    // Action
    ENTRY(nullptr, action::Set, nullptr, "action", nullptr),
#if defined (NODE_SHOWFILE) 
    ENTRY(nullptr, action::SetShowFile, nullptr, "action/showfile", nullptr),
#endif
    // Config
    ENTRY(config::GetDirectory, nullptr, nullptr, "config/directory", nullptr),
    ENTRY(config::GetRemoteConfig, config::SetRemoteConfig, nullptr, "config/remote", "Remote configuration"),
    ENTRY(config::GetGlobal, config::SetGlobal, nullptr, "config/global", "Global"),
    ENTRY(config::GetNetwork, config::SetNetwork, nullptr, "config/network", "Network"),
#if defined(DISPLAY_UDF)
    ENTRY(config::GetDisplayUdf, config::SetDisplayUdf, nullptr, "config/display", "Display"),
#endif    
     // Config Node
#if defined (DMXNODE_TYPE_ARTNET) || defined (DMXNODE_TYPE_E131)     
	ENTRY(config::GetDmxNode, config::SetDmxNode, nullptr, "config/dmxnode", "DMX Node"),
#if defined (DMXNODE_TYPE_ARTNET) 	
	ENTRY(config::GetArtNet, config::SetArtNet, nullptr, "config/artnet", "Art-Net"),
#endif	
#if  defined (DMXNODE_TYPE_E131) || (ARTNET_VERSION >= 4)
	ENTRY(config::GetE131, config::SetE131, nullptr, "config/e131", "sACN E1.31"),
#endif
#endif
#if defined(NODE_OSC_CLIENT)
	ENTRY(config::GetOscClient, config::SetOscClient, nullptr, "config/oscclient", "OSC Client"),
#endif
#if defined(NODE_OSC_SERVER)
	ENTRY(config::GetOscServer, config::SetOscServer, nullptr, "config/oscserver", "OSC Server"),
#endif
#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
	ENTRY(config::GetRdmDevice, config::SetRdmDevice, nullptr, "config/rdmdevice", "RDM Device"),
#if defined(CONFIG_RDM_ENABLE_SENSORS)
	ENTRY(config::GetRdmSensors, config::SetRdmSensors, nullptr, "config/rdmsensors", "RDM Sensors"),
#endif	
#endif	
    // Config Output
#if defined (DMXNODE_OUTPUT_DMX)
    ENTRY(config::GetDmxSend, config::SetDmxSend, nullptr, "config/dmxsend", "DMX Transmit"),
#endif
#if defined(DMXNODE_OUTPUT_PCA9685)
    ENTRY(config::GetPca9685Dmx, config::SetPca9685Dmx, nullptr, "config/dmxpca9685", "DMX PCA9685"),
#endif
#if defined(OUTPUT_DMX_TLC59711)
    ENTRY(config::GetTlc59711Dmx, config::SetTlc59711Dmx, nullptr, "config/dmxtlc59711", "DMX TLC59711"),
#endif
#if defined (DMXNODE_OUTPUT_PIXEL)
    ENTRY(config::GetPixelDmx, config::SetPixelDmx, nullptr, "config/dmxpixel", "DMX Pixel"),
#endif
#if defined (OUTPUT_DMX_MONITOR)
	ENTRY(config::GetDmxMonitor, config::SetDmxMonitor, nullptr, "config/dmxmonitor", "DMX Monitor"),
#endif

#if defined(DMXNODE_OUTPUT_SERIAL)
	ENTRY(config::GetDmxSerial, config::SetDmxSerial, nullptr, "config/dmxserial", "DMX Serial"),
#endif
#if defined(OUTPUT_RGB_PANEL)
	ENTRY(config::GetRgbPanel, config::SetRgbPanel, nullptr, "config/rgbpanel", "RGB Panel"),
#endif
#if defined(OUTPUT_DMX_STEPPER)
	ENTRY(config::GetSparkFunDmx, config::SetSparkFunDmx, nullptr, "config/sparkfundmx", "SparkFun DMX (global)"),
	ENTRY(config::GetDmxL6470Motor0, config::SetDmxL6470Motor0, nullptr, "config/dmxl6470/0", "DMX L6470-0"),
	ENTRY(config::GetDmxL6470Motor1, config::SetDmxL6470Motor1, nullptr, "config/dmxl6470/1", "DMX L6470-1"),
	ENTRY(config::GetDmxL6470Motor2, config::SetDmxL6470Motor2, nullptr, "config/dmxl6470/2", "DMX L6470-2"),
	ENTRY(config::GetDmxL6470Motor3, config::SetDmxL6470Motor3, nullptr, "config/dmxl6470/3", "DMX L6470-3"),
	ENTRY(config::GetDmxL6470Motor4, config::SetDmxL6470Motor4, nullptr, "config/dmxl6470/4", "DMX L6470-4"),
	ENTRY(config::GetDmxL6470Motor5, config::SetDmxL6470Motor5, nullptr, "config/dmxl6470/5", "DMX L6470-5"),
	ENTRY(config::GetDmxL6470Motor6, config::SetDmxL6470Motor6, nullptr, "config/dmxl6470/6", "DMX L6470-6"),
	ENTRY(config::GetDmxL6470Motor7, config::SetDmxL6470Motor7, nullptr, "config/dmxl6470/7", "DMX L6470-7"),
#endif
	// LTC SMPTE
#if defined (NODE_LTC_SMPTE)
	ENTRY(config::GetLtc, config::SetLtc, nullptr, "config/ltc", "LTC SMPTE"),
	ENTRY(config::GetLtcDisplay, config::SetLtcDisplay, nullptr, "config/ltcdisplays", "LTC Displays"),
	ENTRY(config::GetGps, config::SetGps, nullptr, "config/gps", "GPS"),
	ENTRY(config::GetTcNet, config::SetTcNet, nullptr, "config/tcnet", "TCNet"),
	ENTRY(config::GetLtcEtc, config::SetLtcEtc, nullptr, "config/etc", "ETC Connect"),
#endif
    // Config Features
#if defined (NODE_SHOWFILE)    
    ENTRY(config::GetShowFile, config::SetShowFile, nullptr, "config/showfile", "Showfile"),
    ENTRY(ShowFileDirectory, config::SetShowFile, nullptr, "showfile/directory", nullptr),
#endif
#if !defined(DISABLE_FS)
	ENTRY(storage::GetDirectory, nullptr, nullptr, "storage/directory", nullptr)
#endif	
};

constexpr size_t kFileInfosSize = sizeof(kFileInfos) / sizeof(kFileInfos[0]);

static_assert(HasUniqueHashes(kFileInfos, kFileInfosSize), "Duplicate filename hashes detected in kFileInfos!");
} // namespace json