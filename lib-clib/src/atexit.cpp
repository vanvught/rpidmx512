/*
 * atexit.cpp
 */

extern "C" int atexit([[maybe_unused]] void (*func)()) {
    return 0; // No-op
}
