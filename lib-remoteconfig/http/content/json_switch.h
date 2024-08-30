#ifndef JSON_SWITCH_H_
#define JSON_SWITCH_H_

#include <cstdint>

namespace http {
namespace json {
namespace get {
static constexpr uint16_t LIST        = 0x1661;
static constexpr uint16_t VERSION     = 0x6c4b;
static constexpr uint16_t UPTIME      = 0xb7d9;
static constexpr uint16_t DISPLAY     = 0x479b;
static constexpr uint16_t DIRECTORY   = 0x11fa;
static constexpr uint16_t RDM         = 0xa528;
static constexpr uint16_t QUEUE       = 0xb68a;
static constexpr uint16_t TOD         = 0xaf0c;
static constexpr uint16_t PHYSTATUS   = 0xb63a;
static constexpr uint16_t PORTSTATUS  = 0x394e;
static constexpr uint16_t VLANTABLE   = 0xe4be;
static constexpr uint16_t STATUS      = 0x8d49;
static constexpr uint16_t TIMEDATE    = 0x2472;
static constexpr uint16_t RTCALARM    = 0x817b;
static constexpr uint16_t POLLTABLE   = 0x0864;
static constexpr uint16_t TYPES       = 0x5e5a;
}
}
}

namespace http {
inline uint16_t get_uint(const char *pString) {					/* djb2 */
	uint16_t hash = 5381;										/* djb2 */
	uint16_t c;													/* djb2 */
																/* djb2 */
	while ((c = *pString++) != '\0') {							/* djb2 */
		hash = static_cast<uint16_t>(((hash << 5) + hash) + c);	/* djb2 */
	}															/* djb2 */
																/* djb2 */
	return hash;												/* djb2 */
}																/* djb2 */
}

#endif /* JSON_SWITCH_H_ */
