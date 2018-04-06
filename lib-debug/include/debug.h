#ifndef DEBUG_H_
#define DEBUG_H_

#ifndef NDEBUG
#include <stdio.h>
#define DEBUG_ENTRY		printf("--> %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
#define DEBUG_EXIT		printf("<-- %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
#define DEBUG1_ENTRY	printf("\t--> %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
#define DEBUG1_EXIT		printf("\t<-- %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
#define DEBUG2_ENTRY	printf("\t\t--> %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
#define DEBUG2_EXIT		printf("\t\t<-- %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
#else
#define DEBUG_ENTRY
#define DEBUG_EXIT
#define DEBUG1_ENTRY
#define DEBUG1_EXIT
#define DEBUG2_ENTRY
#define DEBUG2_EXIT
#endif

#ifdef NDEBUG
#define DEBUG_PRINTF(FORMAT, ...) ((void)0)
#define DEBUG_PUTS(MSG) ((void)0)
#else
#if defined (__linux__) || defined (__CYGWIN__)
#define DEBUG_PRINTF(FORMAT, ...) \
    fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
        __func__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define DEBUG_PRINTF(FORMAT, ...) \
    printf("%s() in %s, line %i: " FORMAT "\n", \
        __func__, __FILE__, __LINE__, __VA_ARGS__)
#endif	
#define DEBUG_PUTS(MSG) DEBUG_PRINTF("%s", MSG)
#endif

#endif /* DEBUG_H_ */
