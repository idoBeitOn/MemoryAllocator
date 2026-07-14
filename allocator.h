#include <stddef.h>
#include "Stats.h"
#include "BlockHeader.h"

typedef struct Allocator
{
    uint8_t* heapStart;//Stores the address of the beginning of the heap.
    BlockHeader* firstBlock;
    AllocatorStats stats;
} Allocator;



void* my_malloc(size_t size);
void my_free(void* ptr);


void* generateMemoryBlock(size_t size);
AllocatorStats* getStats();
void findSpaceAndAdd(size_t size, BlockHeader** block, BlockHeader** smallestBlock, BlockHeader** lastBlock);
BlockHeader* findLastBlock();
int calculateMustHaveSize(size_t size, BlockHeader* smallestBlock, AllocatorStats* stats, BlockHeader* lastBlock);
BlockHeader* initializeNewBlock(BlockHeader* smallestBlock, size_t size, int remainingSize);



