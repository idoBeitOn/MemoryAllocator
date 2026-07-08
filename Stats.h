#include <stdint.h>
#include <stdbool.h>
typedef struct AllocatorStats
{
    uint8_t magicalBytes;
    bool isLocked;
    uint16_t numOfPages;
    uint32_t numOfBlocks;
} AllocatorStats;  