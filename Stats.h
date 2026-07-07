#include <stdint.h>
typedef struct AllocatorStats
{
    int magicalBytes;
    bool isLocked;
    uint16_t numOfPages;
    uint32_t numOfBlocks;
} AllocatorStats;  