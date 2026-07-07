#include <stddef.h>
#include "Stats.h"
#include "BlockHeader.h"

typedef struct Allocator
{
    uint8_t* heapStart;
    BlockHeader* firstBlock;
    AllocatorStats stats;
} Allocator;



void* my_malloc(size_t size);
void my_free(void* ptr);
