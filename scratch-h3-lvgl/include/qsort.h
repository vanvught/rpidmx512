


#ifndef _QSORT_H
#define _QSORT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*__compar_fn_t) (const void *, const void *);

extern void qsort (void *const pbase, size_t total_elems, size_t size, __compar_fn_t cmp);

#ifdef __cplusplus
}
#endif

#endif
