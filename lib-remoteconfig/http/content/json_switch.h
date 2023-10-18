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
static constexpr uint16_t PHYSTATUS   = 0xb63a;
static constexpr uint16_t PORTSTATUS  = 0x394e;
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
