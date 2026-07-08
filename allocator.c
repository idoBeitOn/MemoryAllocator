#include "allocator.h"
#include "allocator_config.h"
#include <unistd.h>
static Allocator allocator;

void* my_malloc(size_t size) 
{
    if(allocator.heapStart == NULL)//If this is the first allocation, 
                                   //request memory from OS.
    {
        allocator.heapStart = sbrk(0);
        sbrk(PAGE_SIZE);
    }


    char* heapEnd = (char*)sbrk(0);
    long int length = heapEnd - allocator.heapStart;
    if(*(allocator.heapStart) != MAGIC_BYTE)
    {
        *(allocator.heapStart) = MAGIC_BYTE;
        AllocatorStats* stats = (AllocatorStats*)(allocator.heapStart);
        stats->magicalBytes = MAGIC_BYTE;
        stats->numOfPages = 1;
        stats->numOfBlocks = 1;
        BlockHeader* firstBlock = (BlockHeader*)(allocator.heapStart + sizeof(AllocatorStats));
        firstBlock->marker = BLOCK_MARKER;
        firstBlock->inUse = false;
        firstBlock->length = length - sizeof(AllocatorStats) - sizeof(BlockHeader);
        firstBlock->prev = NULL;
        firstBlock->next = NULL;
        allocator.firstBlock = firstBlock;

    }



}


