/**
 * @file configurationstore.h
 *
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

#ifndef CONFIGURATIONSTORE_H_
#define CONFIGURATIONSTORE_H_

#include <cstdint>
#include <cstddef>

#if defined(_MSC_VER)
#pragma pack(push, 1)
#define PACKED
#elif defined(__GNUC__) || defined(__clang__)
#define PACKED __attribute__((packed))
#else
#pragma pack(1)
#define PACKED
#endif

namespace common::store
{

inline constexpr size_t kGlobalSize = 16;
inline constexpr size_t kRemoteConfigSize = 32;
inline constexpr size_t kNetworkSize = 96;
inline constexpr size_t kDisplaySize = 48;
inline constexpr size_t kDmxNodeSize = 212;
inline constexpr size_t kOscClientSize = 912;
inline constexpr size_t kOscServerSize = 400;
inline constexpr size_t kDmxSendSize = 16;
inline constexpr size_t kDmxL6470Size = 848;
inline constexpr size_t kDmxLedSize = 64;
inline constexpr size_t kDmxPwmSize = 24;
inline constexpr size_t kDmxSerialSize = 24;
inline constexpr size_t kDmxMonitorSize = 16;
inline constexpr size_t kRdmDeviceSize = 48;
inline constexpr size_t kRdmSensorsSize = 68;
inline constexpr size_t kRdmSubdevicesSize = 84;
inline constexpr size_t kShowSize = 16;
inline constexpr size_t kLtcSize = 48;
inline constexpr size_t kLtcDisplaySize = 48;
inline constexpr size_t kLtcEtcSize = 20;
inline constexpr size_t kTcNetSize = 16;
inline constexpr size_t kGpsSize = 16;
inline constexpr size_t kMidiSize = 16;
inline constexpr size_t kRgbPanelSize = 16;
inline constexpr size_t kWidgetSize = 16;

struct Global
{
    int32_t utc_offset;
    uint8_t reserved[12];
} PACKED;

static_assert(sizeof(Global) == kGlobalSize);

namespace remoteconfig
{
inline constexpr uint32_t kDisplayNameLength = 24;
}

struct RemoteConfig
{
    uint32_t flags;
    uint8_t reserved[4];
    uint8_t display_name[remoteconfig::kDisplayNameLength];
} PACKED;

static_assert(sizeof(RemoteConfig) == kRemoteConfigSize);

namespace network
{
inline constexpr uint32_t kHostnameSize = 64;

struct Flags
{
    enum class Flag : uint32_t
    {
        kUseStaticIp = (1U << 0),
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace network

struct Network
{
    uint32_t flags;
    uint32_t local_ip;
    uint32_t netmask;
    uint32_t gateway_ip;
    uint32_t name_server_ip;
    uint32_t ntp_server_ip;
    uint8_t host_name[network::kHostnameSize];
    uint8_t reserved[8];
} PACKED;

static_assert(sizeof(Network) == kNetworkSize);

namespace displayudf
{
inline constexpr uint32_t kLabelIndexSize = 28;

struct Flags
{
    enum class Flag : uint32_t
    {
        kFlipVertically = (1U << 0),
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace displayudf

struct DisplayUdf
{
    uint32_t flags;
    uint8_t label_index[displayudf::kLabelIndexSize];
    uint8_t sleep_timeout;
    uint8_t intensity;
    uint8_t reserved[14];
} PACKED;

static_assert(sizeof(DisplayUdf) == kDisplaySize);

namespace dmxnode
{
inline constexpr uint32_t kParamPorts = 4;
inline constexpr uint32_t kNodeNameLength = 64;
inline constexpr uint32_t kLabelNameLength = 18;

struct Flags
{
    enum class Flag : uint32_t
    {
        kEnableRdm = (1U << 0),
        kMapUniverse0 = (1U << 1),
        kDisableMergeTimeout = (1U << 2)
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace dmxnode

struct DmxNode
{
    uint32_t flags;
    uint8_t personality;
    uint8_t reserved;
    uint16_t universe[dmxnode::kParamPorts];
    uint16_t direction;
    uint16_t merge_mode;
    uint8_t output_style;
    uint8_t fail_safe;
    uint8_t long_name[dmxnode::kNodeNameLength];
    uint8_t label[dmxnode::kParamPorts][dmxnode::kLabelNameLength];
    uint8_t reserved1[2];
    uint16_t protocol;
    uint16_t rdm;
    uint32_t destination_ip[dmxnode::kParamPorts];
    uint8_t reserved2[4];
    uint8_t priority[dmxnode::kParamPorts];
    uint8_t reserved3[4];
    uint8_t reserved4[22];
} PACKED;

static_assert(offsetof(DmxNode, universe) % alignof(uint16_t) == 0, "universe must be uint16_t-aligned");
static_assert(offsetof(DmxNode, protocol) % alignof(uint16_t) == 0, "protocol must be uint16_t-aligned");
static_assert(sizeof(DmxNode) == kDmxNodeSize);

namespace osc::client
{
inline constexpr uint32_t kCmdCount = 8;
inline constexpr uint32_t kCmdPathLength = 64;
inline constexpr uint32_t kLedCount = 8;
inline constexpr uint32_t kLedPathLength = 48;

struct Flags
{
    enum class Flag : uint32_t
    {
        kPingDisable = (1U << 0),
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace osc::client

struct OscClient
{
    uint32_t flags;
    uint16_t outgoing_port;
    uint16_t incoming_port;
    uint32_t server_ip;
    uint8_t ping_delay;
    uint8_t reserved[3];
    char cmd[osc::client::kCmdCount][osc::client::kCmdPathLength];
    char led[osc::client::kLedCount][osc::client::kLedPathLength];
} PACKED;

static_assert(sizeof(OscClient) == kOscClientSize);

namespace osc::server
{
inline constexpr uint32_t kPathLength = 128;

struct Flags
{
    enum class Flag : uint32_t
    {
        kPartialTransmission = (1U << 0),
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace osc::server

struct OscServer
{
    uint32_t flags;
    uint16_t outgoing_port;
    uint16_t incoming_port;
    uint8_t output_type;
    uint8_t reserved1[3];
    uint8_t reserved2[4];
    char path[osc::server::kPathLength];
    char path_info[osc::server::kPathLength];
    char path_blackout[osc::server::kPathLength];
} PACKED;

static_assert(sizeof(OscServer) == kOscServerSize);

struct DmxSend
{
    uint32_t flags;
    uint16_t break_time;
    uint16_t mab_time;
    uint8_t refresh_rate;
    uint8_t slots_count;
    uint8_t reserved2[6];
} PACKED;

static_assert(offsetof(DmxSend, break_time) % alignof(uint16_t) == 0, "break_time must be uint16_t-aligned");
static_assert(sizeof(DmxSend) == kDmxSendSize);

namespace dmxled
{

inline constexpr uint32_t kMaxUniverses = 16;

struct Flags
{
    enum class Flag : uint32_t
    {
        kEnableGamma = (1U << 0),
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace dmxled

struct DmxLed
{
    uint32_t flags;
    uint8_t type;
    uint8_t map;
    uint16_t count;
    uint16_t grouping_count;
    uint16_t dmx_start_address;
    uint8_t reserved1[4];
    uint32_t spi_speed_hz;
    uint8_t global_brightness;
    uint8_t active_outputs;
    uint8_t test_pattern;
    uint8_t gamma_value;
    uint8_t low_code;
    uint8_t high_code;
    uint8_t reserved2[6];
    uint16_t start_universe[dmxled::kMaxUniverses];
} PACKED;

static_assert(offsetof(DmxLed, count) % alignof(uint16_t) == 0, "count must be uint16_t-aligned");
static_assert(offsetof(DmxLed, spi_speed_hz) % alignof(uint32_t) == 0, "spi_speed_hz must be uint32_t-aligned");
static_assert(offsetof(DmxLed, start_universe) % alignof(uint16_t) == 0, "start_universe must be uint16_t-aligned");
static_assert(sizeof(DmxLed) == kDmxLedSize);

namespace dmxpwm
{
struct Flags
{
    enum class Flag : uint32_t
    {
        kModeServo = 1U << 0,
        kUse8Bit = 1U << 1,
        kLedOutputInvert = 1U << 2,
        kLedOutputOpendrain = 1U << 3,
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace dmxpwm

struct DmxPwm
{
    uint32_t flags;
    uint8_t address;
    uint8_t reserved1;
    uint16_t channel_count;
    uint16_t dmx_start_address;
    uint16_t led_pwm_frequency;
    uint16_t servo_left_us;
    uint16_t servo_center_us;
    uint16_t servo_right_us;
    uint8_t reserved2[6];
} PACKED;

static_assert(sizeof(DmxPwm) == kDmxPwmSize);

struct DmxSerial
{
    uint32_t set_list;
    uint8_t type;
    uint8_t reserved1[3];
    uint32_t baud;
    uint8_t bits;
    uint8_t parity;
    uint8_t stop_bits;
    uint8_t reserved2;
    uint32_t spi_speed_hz;
    uint8_t spi_mode;
    uint8_t i2c_address;
    uint8_t i2c_speed_mode;
    uint8_t reserved3;
} PACKED;

static_assert(offsetof(DmxSerial, baud) % alignof(uint32_t) == 0, "baud must be uint32_t-aligned");
static_assert(offsetof(DmxSerial, spi_speed_hz) % alignof(uint32_t) == 0, "spi_speed_hz must be uint32_t-aligned");
static_assert(sizeof(DmxSerial) == kDmxSerialSize);

struct DmxMonitor
{
    uint32_t set_list;
    uint16_t dmx_start_address;
    uint16_t dmx_max_channels;
    uint8_t format;
    uint8_t reserved[7];
} PACKED;

static_assert(sizeof(DmxMonitor) == kDmxMonitorSize);

namespace rdmdevice
{
inline constexpr uint32_t kLabelMaxLength = 32;
}

struct RdmDevice
{
    uint32_t set_list;
    uint8_t device_root_label[rdmdevice::kLabelMaxLength];
    uint8_t device_root_label_length;
    uint8_t reserved;
    uint16_t product_category;
    uint16_t product_detail;
    uint8_t reserved2[6];
} PACKED;

static_assert(offsetof(RdmDevice, product_category) % alignof(uint16_t) == 0, "product_category must be uint16_t-aligned");
static_assert(sizeof(RdmDevice) == kRdmDeviceSize);

namespace rdm::sensors
{
inline constexpr uint32_t kMaxSensors = 16;
inline constexpr uint32_t kMaxDevices = 8;
} // namespace rdm::sensors

struct RdmSensors
{
    uint32_t devices;
    struct Entry
    {
        uint8_t type;
        uint8_t address;
        uint8_t reserved[2];
    } PACKED entry[rdm::sensors::kMaxDevices];
    int16_t calibrate[rdm::sensors::kMaxSensors];
} PACKED;

static_assert(offsetof(RdmSensors, calibrate) % alignof(uint16_t) == 0, "calibrate must be uint16_t-aligned");
static_assert(sizeof(RdmSensors) == kRdmSensorsSize);

namespace rdm::subdevices
{
inline constexpr uint32_t kMaxSubdevices = 8;
}

struct RdmSubdevices
{
    uint32_t count;
    struct Entry
    {
        uint8_t type;
        uint8_t chip_select;
        uint8_t address;
        uint8_t reserved;
        uint32_t speed_hz;
        uint16_t dmx_start_address;
    } PACKED entry[rdm::subdevices::kMaxSubdevices];
} PACKED;

static_assert(offsetof(RdmSubdevices::Entry, speed_hz) % alignof(uint32_t) == 0, "speed_hz must be uint32_t-aligned");
static_assert(offsetof(RdmSubdevices::Entry, dmx_start_address) % alignof(uint16_t) == 0, "dmx_start_address must be uint16_t-aligned");
static_assert(sizeof(RdmSubdevices) == kRdmSubdevicesSize);

namespace showfile
{
struct Flags
{
    enum class Flag : uint32_t
    {
        kOptionAutoPlay = 1U << 0,
        kOptionLoop = 1U << 1,
        kOptionDisableSync = 1U << 2,
        kOptionArtnetDisableUnicast = 1U << 3,
        kOptionSacnSyncUniverse = 1U << 4
    };
};
} // namespace showfile

struct ShowFile
{
    uint32_t flags;
    uint8_t show;
    uint8_t reserved;
    uint16_t osc_port_incoming;
    uint16_t osc_port_outgoing;
    uint16_t universe;
    uint8_t reserved2[2];
    uint8_t disable_unicast;
    uint8_t dmx_master;
} PACKED;

static_assert(offsetof(ShowFile, osc_port_incoming) % alignof(uint16_t) == 0, "osc_port_incoming must be uint16_t-aligned");
static_assert(sizeof(ShowFile) == kShowSize);

namespace ltc
{
struct Flags
{
    enum class Flag : uint32_t
    {
        kNtpEnable = 1U << 0,
        kAutoStart = 1U << 1,
        kIsAltFuntion = 1U << 2,
        kSkipFree = 1U << 3,
        kShowSystime = 1U << 4,
        kTimeSyncDisabled = 1U << 5,
        kOscEnabled = 1U << 6,
        kGpsStart = 1U << 7,
        kIgnoreStart = 1U << 8,
        kIgnoreStop = 1U << 9,
        kWS28xxEnable = 1U << 10,
        kRgbpanelEnable = 1U << 11,
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace ltc

struct Ltc
{
    uint32_t flags;
    uint8_t source;
    uint8_t volume;
    uint8_t disabled_outputs;
    uint8_t ntp_year;
    uint8_t ntp_month;
    uint8_t ntp_day;
    uint8_t fps;
    uint8_t start_frame;
    uint8_t start_second;
    uint8_t start_minute;
    uint8_t start_hour;
    uint8_t stop_frame;
    uint8_t stop_second;
    uint8_t stop_minute;
    uint8_t stop_hour;
    uint8_t reserved6;
    uint8_t rgb_led_type;
    uint8_t reserved7;
    uint8_t skip_seconds;
    uint8_t reserved8;
    uint8_t reserved1[12];
    uint8_t reserved2[2];
    uint16_t osc_port;
    int32_t utc_offset;
    uint32_t time_code_ip;
} PACKED;

static_assert(offsetof(Ltc, osc_port) % alignof(uint16_t) == 0, "osc_port must be uint16_t-aligned");
static_assert(sizeof(Ltc) == kLtcSize);

namespace ltc::display
{
inline constexpr uint32_t kMaxColours = 6;
inline constexpr uint32_t kMaxInfoMessage = 8;

struct Flags
{
    enum class Flag : uint32_t
    {
        kRotaryFullStep = 1U << 0,
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace ltc::display

struct LtcDisplay
{
    uint32_t flags;
    uint8_t max7219_type;
    uint8_t max7219_intensity;
    uint8_t ws28xx_type;
    uint8_t ws28xx_rgb_mapping;
    uint8_t ws28xx_display_type;
    uint8_t reserved1;
    uint8_t display_rgb_intensity;
    uint8_t display_rgb_colon_blink_mode;
    uint32_t display_rgb_colour[ltc::display::kMaxColours];
    char info_message[ltc::display::kMaxInfoMessage];
    uint8_t oled_intensity;
    uint8_t reserved2[3];
} PACKED;

static_assert(offsetof(LtcDisplay, display_rgb_colour) % alignof(uint32_t) == 0, "display_rgb_colour must be uint32_t-aligned");
static_assert(sizeof(LtcDisplay) == kLtcDisplaySize);

struct LtcEtc
{
    uint32_t set_list;
    uint32_t destination_ip;
    uint32_t source_multicast_ip;
    uint16_t destination_port;
    uint16_t source_port;
    uint8_t udp_terminator;
    uint8_t reserved1[3];
} PACKED;

static_assert(sizeof(LtcEtc) == kLtcEtcSize);

namespace tcnet
{
inline constexpr uint32_t kNodeNameLength = 8;

struct Flags
{
    enum class Flag : uint32_t
    {
        kUseTimecode = 1U << 0,
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace tcnet

struct TcNet
{
    uint32_t flags;
    char node_name[tcnet::kNodeNameLength];
    uint8_t layer;
    uint8_t time_code_type;
    uint8_t reserved[2];
} PACKED;

static_assert(sizeof(TcNet) == kTcNetSize);

namespace gps
{
struct Flags
{
    enum class Flag : uint32_t
    {
        kEnable = 1U << 0,
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace gps

struct Gps
{
    uint32_t flags;
    int32_t utc_offset;
    uint8_t module;
    uint8_t reserved[7];
} PACKED;

static_assert(sizeof(Gps) == kGpsSize);

namespace midi {
struct Flags
{
    enum class Flag : uint32_t
    {
        kActiveSense = 1U << 0,
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
}  // namespace midi

struct Midi
{
    uint32_t flags;
    uint32_t baudrate;
    uint8_t reserved[8];
} PACKED;

static_assert(sizeof(Midi) == kMidiSize);

struct RgbPanel
{
    uint32_t set_list;
    uint8_t cols;
    uint8_t rows;
    uint8_t chain;
    uint8_t type;
    uint8_t reserved[8];
} PACKED;

static_assert(sizeof(RgbPanel) == kRgbPanelSize);

struct Widget
{
    uint32_t set_list;
    uint8_t break_time;
    uint8_t mab_time;
    uint8_t refresh_rate;
    uint8_t mode;
    uint8_t throttle;
    uint8_t reserved[7];
} PACKED;

static_assert(sizeof(Widget) == kWidgetSize);

namespace l6470dmx
{
inline constexpr uint32_t kMaxMotors = 8;

namespace sparkfun
{
struct Flags
{
    enum class Flag : uint32_t
    {
        kIsSetPosition = 1U << 0,
        kIsSetSpiCs = 1U << 1,
        kIsSetResetPin = 1U << 2,
        kIsSetBusyPin = 1U << 3,
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace sparkfun

struct SparkFun
{
    uint32_t flags;
    uint8_t position;
    uint8_t spi_cs;
    uint8_t reset_pin;
    uint8_t busy_pin;
    uint8_t reserved[8];
} PACKED;

struct SlotInfo
{
    uint16_t category;
    uint8_t type;
    uint8_t reserved;
};

namespace mode
{
inline constexpr uint16_t kMaxDmxFootprint = 4;
	
struct Flags
{
    enum class Flag : uint32_t
    {
        kUseSwitch = 1U << 0,
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};
} // namespace mode

struct Mode
{
    uint32_t flags;
    uint8_t dmx_mode;
    uint8_t reserved1;
    uint16_t dmx_start_address;
    uint32_t max_steps;
    uint32_t switch_steps_per_sec;
    uint8_t switch_action;
    uint8_t switch_dir;
    uint8_t reserved2[2];
    SlotInfo slot_info[mode::kMaxDmxFootprint];
} PACKED;

namespace l6470
{
struct Flags
{
    enum class Flag : uint32_t
    {
        kIsSetMinSpeed = 1U << 0,
        kIsSetMaxSpeed = 1U << 1,
        kIsSetAcc = 1U << 2,
        kIsSetDec = 1U << 3,
        kIsSetKvalHold = 1U << 4,
        kIsSetKvalRun = 1U << 5,
        kIsSetKvalAcc = 1U << 6,
        kIsSetKvalDec = 1U << 7,
        kIsSetMicroSteps = 1U << 8
    };

    static constexpr bool Has(uint32_t value, Flag flag) noexcept { return (value & static_cast<uint32_t>(flag)) != 0; }
};	
}  // namespace l6470

struct L6470
{
    uint32_t flags;
    uint32_t min_speed;
    uint32_t max_speed;
    uint32_t acc;
    uint32_t dec;
    uint8_t kval_hold;
    uint8_t kval_run;
    uint8_t kval_acc;
    uint8_t kval_dec;
    uint8_t micro_steps;
    uint8_t reserved[3];
} PACKED;

struct Motor
{
    uint32_t set_list;
    float step_angel;
    float voltage;
    float current;
    float resistance;
    float inductance;
} PACKED;

struct Store
{
    l6470dmx::SparkFun spark_fun;
    l6470dmx::Mode mode;
    l6470dmx::L6470 l6470;
    l6470dmx::Motor motor;
} PACKED;

static_assert(offsetof(Store, mode) % alignof(uint32_t) == 0, "mode must be uint32_t-aligned");
static_assert(offsetof(Store, l6470) % alignof(uint32_t) == 0, "l6470 must be uint32_t-aligned");
static_assert(offsetof(Store, motor) % alignof(uint32_t) == 0, "motor must be uint32_t-aligned");
}  // namespace l6470dmx

struct DmxL6470
{
    l6470dmx::SparkFun spark_fun_global;
    l6470dmx::Store store[l6470dmx::kMaxMotors];
} PACKED;

static_assert(offsetof(DmxL6470, store) % alignof(uint32_t) == 0, "store must be uint32_t-aligned");
static_assert(sizeof(DmxL6470) == kDmxL6470Size);

} // namespace common::store

namespace configurationstore
{
inline constexpr uint32_t kMagicNumberSize = 4;
inline constexpr uint32_t kVersionSize = 2;
} // namespace configurationstore

struct ConfigurationStore
{
    uint8_t magic_number[configurationstore::kMagicNumberSize];
    uint8_t version[configurationstore::kVersionSize];
    uint8_t reserved[10];

    common::store::Global global;
    common::store::RemoteConfig remote_config;
    common::store::Network network;
    common::store::DisplayUdf display_udf;
    common::store::DmxNode dmx_node;
    common::store::OscClient osc_client;
    common::store::OscServer osc_server;
    common::store::DmxSend dmx_send;
    common::store::DmxL6470 dmx_l6470;
    common::store::DmxLed dmx_led;
    common::store::DmxPwm dmx_pwm;
    common::store::DmxSerial dmx_serial;
    common::store::DmxMonitor dmx_monitor;
    common::store::RdmDevice rdm_device;
    common::store::RdmSensors rdm_sensors;
    common::store::RdmSubdevices rdm_subdevices;
    common::store::ShowFile show_file;
    common::store::Ltc ltc;
    common::store::LtcDisplay ltc_display;
    common::store::LtcEtc ltc_etc;
    common::store::TcNet tcnet;
    common::store::Gps gps;
    common::store::Midi midi;
    common::store::RgbPanel rgb_panel;
    common::store::Widget widget;
} PACKED;

static_assert(offsetof(ConfigurationStore, global) == 16, "Wrong offset: global");

#if defined(_MSC_VER)
#pragma pack(pop)
#elif defined(__GNUC__) || defined(__clang__)
#undef PACKED
#else
#pragma pack()
#undef PACKED
#endif

#endif  // CONFIGURATIONSTORE_H_
