/*
 * atexit.cpp
 */

extern "C" int atexit([[maybe_unused]] void (*func)(void)) {
    return 0; // No-op
}
