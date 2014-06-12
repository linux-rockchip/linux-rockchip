
#if 0



#include "include.h"

void* OSW_MemAlloc(UINT32 size)
{
    void* ret = (void*)RKmalloc(size);
    if(ret)
        RKmemset(ret,0,size);
	return ret;
}

void OSW_MemFree(void* pMemToFree)
{
	
	RKfree(pMemToFree);
	pMemToFree =NULL;
}

#endif //jan


