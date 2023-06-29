/*
 * rdm_manufacturer_pid.h
 */

#ifndef RDM_MANUFACTURER_PID_H_
#define RDM_MANUFACTURER_PID_H_

#include <cstddef>

#if  ! defined (PACKED)
#define PACKED __attribute__((packed))
#endif

namespace rdm {
struct ParameterDescription {
	const uint16_t pid;
	const uint8_t pdl_size;
	const uint8_t data_type;
	const uint8_t command_class;
	const uint8_t type;
	const uint8_t unit;
	const uint8_t prefix;
	const uint32_t min_value;
	const uint32_t default_value;
	const uint32_t max_value;
	const char *description;
	const uint8_t pdl;
}PACKED;

static constexpr uint8_t pdlParameterDescription(const size_t N) {
	return static_cast<uint8_t>(sizeof(ParameterDescription) - sizeof(const char *) - sizeof(const uint8_t) + N);
}

static constexpr uint32_t DEVICE_DESCRIPTION_MAX_LENGTH = 32;

template <uint16_t Value>
struct ManufacturerPid {
    static constexpr uint16_t code = __builtin_bswap16(Value);
    static_assert(code >= __builtin_bswap16(0x8000) && code <= __builtin_bswap16(0xFFDF), "The manufacturer specific PID must be in range 0x8000 to 0xFFDF");
};

template <typename T, size_t N>
struct Description {
    static constexpr size_t size = N - 1;
    static_assert(size <= DEVICE_DESCRIPTION_MAX_LENGTH, "Description is too long");
    static constexpr char const* value = T::description;
};

size_t get_table_size();

struct ManufacturerParamData {
	uint8_t nPdl;
	uint8_t *pParamData;
};

bool handle_manufactureer_pid_get(const uint16_t nPid, const ManufacturerParamData *pIn, ManufacturerParamData *pOut, uint16_t& nReason);

}  // namespace rdm

#endif /* RDM_MANUFACTURER_PID_H_ */
