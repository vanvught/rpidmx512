#include <stdlib.h>

void operator delete(void *pBlock) {
	//free(pBlock);
}

void operator delete[](void *pBlock) {
	//free(pBlock);
}
