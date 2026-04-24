/**
 * @file emac_counters.h
 *
 */

#ifndef EMAC_COUNTERS_H_
#define EMAC_COUNTERS_H_

#include <cstdint>

namespace emac::eth::globals {
struct Counters {
    uint32_t sent;
    uint32_t received;
	uint32_t received_error;
};
extern struct Counters counter;
} // namespace emac::eth::globals

#endif /* EMAC_COUNTERS_H_ */
