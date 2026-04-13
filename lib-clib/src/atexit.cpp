/*
 * atexit.cpp
 */

extern "C" int atexit([[maybe_unused]] void (*func)()) { // NOLINT
    return 0; // No-op
}
