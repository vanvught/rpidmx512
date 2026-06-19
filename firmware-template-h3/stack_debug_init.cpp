#include <stdint.h>

#include "firmware/debug/debug_stack.h"

extern unsigned char stack_low;
extern unsigned char _sp; // NOLINT

extern "C" void stack_debug_init() { // NOLINT
    auto* start = reinterpret_cast<uint32_t*>(&stack_low);
    auto* end = reinterpret_cast<uint32_t*>(&_sp);

    while (start < end) {
        *start = debug::stack::kMagicWord;
        start++;
    }
}