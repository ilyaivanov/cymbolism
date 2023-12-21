#include "types.h"

#define IS_TRACKING_MEMORY

typedef enum CountMetric
{
    MemoryAllocated,
    MemoryFreed,
    NumberOfCountMetrics
} CountMetric;


typedef struct CountingMetrics
{
    i32 hasCalledThisFrame; 
    u64 callCount[NumberOfCountMetrics];
    u64 count[NumberOfCountMetrics];
    u64 totalCount[NumberOfCountMetrics];
} CountingMetrics;


CountingMetrics countMetrics = {0};

inline void Track(i32 size, CountMetric metric)
{
    countMetrics.hasCalledThisFrame = 1;
    countMetrics.callCount[metric]++;
    countMetrics.count[metric] += size;
    countMetrics.totalCount[metric] += size;
}

inline void *AllocateZeroedMemory(int length, size_t size)
{
#ifdef IS_TRACKING_MEMORY
    i32 totalMem = length * size;
    Track(totalMem, MemoryAllocated);

    size_t* res = calloc(1, totalMem + sizeof(size_t));
    *res = totalMem;
    return res+ 1;
#else
    return calloc(length, size);
#endif
};

inline void *AllocateMemory(size_t size)
{
#ifdef IS_TRACKING_MEMORY
    Track(size, MemoryAllocated);

    size_t* res = malloc(size + sizeof(size_t));
    *res = size;
    return res + 1;
#else
    return malloc(size);
#endif
};


inline void FreeMemory(void * ptr)
{
#ifdef IS_TRACKING_MEMORY
    size_t *res = (size_t*)ptr;
    res--;

    Track(*res, MemoryFreed);
    free(res);
#else
    free(ptr);
#endif
};

inline void *VirtualAllocateMemory(size_t size)
{
#ifdef IS_TRACKING_MEMORY
    Track(size, MemoryAllocated);

    size_t* res = VirtualAlloc(0, size + 1, MEM_COMMIT, PAGE_READWRITE);
    *res = size;
    return res + 1;
#else
     return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
#endif
};

inline void VirtualFreeMemory(void * ptr)
{
#ifdef IS_TRACKING_MEMORY
    size_t *res = (size_t*)ptr;
    res--;

    Track(*res, MemoryFreed);
    VirtualFree(res, 0, MEM_RELEASE);
#else
    VirtualFree(ptr, 0, MEM_RELEASE);
#endif
};



inline void ClearFrameData()
{
    countMetrics.callCount[MemoryFreed] = 0;
    countMetrics.count[MemoryFreed] = 0;

    countMetrics.callCount[MemoryAllocated] = 0;
    countMetrics.count[MemoryAllocated] = 0;
}

inline void ReportMemoryChanges()
{
    if(countMetrics.hasCalledThisFrame)
    {
        char buff[512] = {0};

        
        sprintf(buff, "   +%-12lld %d -%-12lld %d. Total: +%-12lld -%-12lld\n",
                countMetrics.count[MemoryAllocated], countMetrics.callCount[MemoryAllocated],
                countMetrics.count[MemoryFreed], countMetrics.callCount[MemoryFreed], 
                countMetrics.totalCount[MemoryAllocated], countMetrics.totalCount[MemoryFreed]);
        OutputDebugStringA(buff);   

        ClearFrameData();

        countMetrics.hasCalledThisFrame = 0;
    }
}

inline void ReportStartupMemory()
{
    char buff[512] = {0};
    sprintf(buff, "Startup allocated: +%-12lld -%-12lld.\n",
            countMetrics.totalCount[MemoryAllocated], countMetrics.totalCount[MemoryFreed]);
    OutputDebugStringA(buff);
    countMetrics.hasCalledThisFrame = 0;

    ClearFrameData();
}