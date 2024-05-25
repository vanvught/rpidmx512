/*
 * impure_prt.cpp
 */

typedef struct _reent {
    int _errno; // Placeholder for the actual contents of _reent
} _reent;

// Define the global _impure_ptr. Normally points to reentrant data.
static struct _reent _reent_data = {0};
struct _reent *_impure_ptr = &_reent_data;
