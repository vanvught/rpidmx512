
typedef int (*__compar_fn_t) (const void *, const void *);

void qsort (void *const pbase, size_t total_elems, size_t size, __compar_fn_t cmp);

