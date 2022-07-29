/**
 * @file pp.h
 *
 */
/*
 *  Universal Discovery Protocol
 *  A UDP protocol for finding Etherdream/Heroic Robotics lighting devices
 *
 *  (c) 2012 Jas Strong and Jacob Potter
 *  <jasmine@electronpusher.org> <jacobdp@gmail.com>
 *
 *	PixelPusherBase/PixelPusherExt split created by Henner Zeller 2016
 *
 *	pusher command stuff added by Christopher Schardt 2017
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PP_H_
#define PP_H_

#include <cstdint>
#include <algorithm>

#include "lightset.h"

#if !defined(LIGHTSET_PORTS)
# error LIGHTSET_PORTS is not defined
#endif

namespace pp {
namespace lightset {
static constexpr uint32_t MAX_PORTS = LIGHTSET_PORTS;
}  // namespace lightset
namespace configuration {
static constexpr uint32_t CHANNELS_PER_PIXEL = 3;
static constexpr uint32_t UNIVERSE_MAX_LENGTH = 510;///< 512 / 3 {CHANNELS_PER_PIXEL} -> 170 * 3 {CHANNELS_PER_PIXEL} = 510
static constexpr uint32_t COUNT_MAX = 480;			///< 1440 / 3 {CHANNELS_PER_PIXEL}
}  // namespace configuration
static constexpr uint16_t UDP_PORT_DISCOVERY = 7331;
static constexpr uint16_t UDP_PORT_DATA = 5078;
namespace version {
static constexpr uint16_t MIN = 121;
static constexpr uint16_t MAX = 150;
}  // namespace version

enum class DeviceType : uint8_t {
	ETHERDREAM = 0, LUMIABRIDGE = 1, PIXELPUSHER = 2, LEDPLAY = 3	///< When a LEDPlay configuration includes a PixelPusher receiver, PIXELPUSHER is used instead
};

enum class PusherFlags : uint32_t {
	PROTECTED			= 0x0001, ///< require qualified registry.getStrips() call.
	FIXEDSIZE			= 0x0002, ///< requires every datagram same size.
	GLOBAL_BRIGHTNESS	= 0x0004, ///< supports PPPusherCommandGlobalBrightness (NOT whether any strip supports hardware brightness)
	STRIP_BRIGHTNESS	= 0x0008, ///< supports PPPusherCommandStripBrightness (NOT whether any strip supports hardware brightness)
	DYNAMICS			= 0x0010, ///< supports PPPusherCommandDynamics, and the functionality IS supported
	CAN_BUFFER			= 0x0020, ///< packets can be sent all at once, not spaced out in time
	_16BITSTUFF			= 0x0040  ///< strip count in strip_count_16 field
};

enum class StripFlags : uint8_t {
	RGBOW			= 0x01,	///< High CRI strip
	WIDEPIXELS		= 0x02,	///< 48 Bit/pixel RGBrgb
	LOGARITHMIC		= 0x04,	///< LED has logarithmic response.
	MOTION			= 0x08,	///< A motion controller.
	NOTIDEMPOTENT	= 0x10,	///< motion controller with side-effects.
	BRIGHTNESS 		= 0x20	///< Strip configured for hardware that supports brightness
};

#if !defined(PACKED)
# define PACKED __attribute__((__packed__))
#endif

struct PixelPusherBase {
	uint8_t strips_attached;	///< if PFLAG_16BITSTUFF, this must be set to 1, causing all strips to have same flags
	uint8_t max_strips_per_packet;
	uint16_t pixels_per_strip;	///< uint16_t used to make alignment work
	uint32_t update_period;		///< in microseconds
	uint32_t power_total;		///< in PWM units
	uint32_t delta_sequence;	///< difference between received and expected sequence numbers
	int32_t controller_ordinal; ///< ordering number for this controller.
	int32_t group_ordinal;      ///< group number for this controller.
	uint16_t artnet_universe;	///< configured artnet starting point for this controller
	uint16_t artnet_channel;
	uint16_t my_port;
	uint16_t padding1_;
	uint8_t strip_flags[8];     ///< flags for each strip, for up to eight strips
} PACKED;

struct PixelPusherExt {
	uint16_t strip_count_16;	///< was "padding2_" before PFLAG_16BITSTUFF
	uint32_t pusher_flags;      ///< flags for the whole pusher
	uint32_t segments;          ///< number of segments in each strip
	uint32_t power_domain;      ///< power domain of this pusher
	uint8_t last_driven_ip[4];  ///< last host to drive this pusher
	uint16_t last_driven_port;  ///< source port of last update
} PACKED;

struct PixelPusher {
	PixelPusherBase base;   	///< Good for up to 8 strips.
	PixelPusherExt ext;
} PACKED;

struct DiscoveryPacketHeader {
	uint8_t mac_address[6];
	uint8_t ip_address[4];  	///< network byte order
	uint8_t device_type;
	uint8_t protocol_version; 	///< for the device, not the discovery
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t hw_revision;
	uint16_t sw_revision;
	uint32_t link_speed;    	///< in bits per second
} PACKED;

struct DiscoveryPacket {
	DiscoveryPacketHeader header;
	PixelPusher pixelpusher;
} PACKED;

namespace command {
enum class Type : uint8_t {
	NONE				= 0x00,	///< returned when data not present
	RESET				= 0x01,
	GLOBAL_BRIGHTNESS	= 0x02,	///< data is 2 bytes for 0xFFFF-normalized brightness
	WIFI_CONFIGURE		= 0x03,	///< WiFi is not supported.
	LED_CONFIGURE		= 0x04,	///< Is not supported.
	STRIP_BRIGHTNESS	= 0x05,	///< data is 1 byte strip index, followed by 2-byte 0xFFFF-normalized brightness
	DYNAMICS			= 0x06,
};
namespace packet {
#if !defined(CONFIG_PP_16BITSTUFF)
struct GlobalBrightness {
	uint32_t sequenceNumber;
	uint8_t magicNumber[16];	///< static constexpr uint8_t COMMAND_MAGIC[16] defined in pp.cpp
	uint8_t commandType;		///< Type::GLOBAL_BRIGHTNESS
	uint16_t brightness;		///< 0xFFFF for 1.0
} PACKED;

struct StripBrightness
{
	uint32_t sequenceNumber;
	uint8_t magicNumber[16];	///< static constexpr uint8_t COMMAND_MAGIC[16] defined in pp.cpp
	uint8_t commandType;		///< Type::STRIP_BRIGHTNESS
	uint8_t stripIndex;
	uint16_t brightness;		///< 0xFFFF for 1.0
}PACKED ;
#else
#endif
}  // namespace packet
}  // namespace command
}  // namespace pp

class PixelPusher {
public:
	PixelPusher();
	~PixelPusher() {
	}

	void SetOutput(LightSet *pLightSet) {
		m_pLightSet = pLightSet;
	}

	LightSet *GetOutput() const {
		return m_pLightSet;
	}

	void SetCount(const uint32_t nCount, const uint32_t nActivePorts, const bool hasGlobalBrightness) {
		m_nCount = std::min(nCount, pp::configuration::COUNT_MAX);
		m_nUniverses = 1 + (m_nCount / (1 + (pp::configuration::UNIVERSE_MAX_LENGTH / pp::configuration::CHANNELS_PER_PIXEL)));
		m_nActivePorts = std::min(nActivePorts, pp::lightset::MAX_PORTS / 3U);
		m_nPortIndexLast = m_nActivePorts * m_nUniverses;
		m_hasGlobalBrightness = hasGlobalBrightness;
	}

	void Start();
	void Stop();
	void Print();

	void Run();

	static PixelPusher *Get() {
		return s_pThis;
	}

private:
	void HandlePusherCommand(const uint8_t *pBuffer, uint32_t nSize);

private:
	uint32_t m_nMillis;
	int32_t m_nHandleDiscovery { -1 };
	int32_t m_nHandleData { -1 };
	uint32_t m_nBytesReceived { 0 };
	uint32_t m_nCount { 0 };
	uint32_t m_nStripDataLength { 0 };
	uint32_t m_nUniverses { 0 };
	uint32_t m_nPortIndexLast { 0 };
	uint32_t m_nActivePorts { 0 };
	bool m_hasGlobalBrightness { false };

	LightSet *m_pLightSet { nullptr };

	pp::DiscoveryPacket m_DiscoveryPacket;
	uint8_t *m_pDataPacket { nullptr };

	static PixelPusher *s_pThis;
};

#endif /* PP_H_ */
