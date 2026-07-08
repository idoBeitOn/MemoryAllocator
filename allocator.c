#include "allocator.h"
#include "allocator_config.h"
#include <unistd.h>
static Allocator allocator;

//Returns a pointer to  memory in the heap.
//Argument is the number of bytes requested by the user.
void* my_malloc(size_t size) 
{
    if(allocator.heapStart == NULL)//If this is the first allocation, 
                                   //initialize the heap.
    {
        allocator.heapStart = sbrk(0);//sbrk returns the current end of the heap.
        sbrk(PAGE_SIZE);//Asking OS to increase the heap size by one page.
    }

    char* heapEnd = (char*)sbrk(0);//Sbrk(0) returns the current end of the heap.
    size_t length = heapEnd - allocator.heapStart;//Calculate the length of the heap in bytes.
                                                    //Both are byte pointers

    if(*(allocator.heapStart) != MAGIC_BYTE)//The first byte of the heap stores a magic value used to identify an initialized allocator.
                                            // If the value is missing, initialize the allocator metadata and first memory block
    {
        *(allocator.heapStart) = MAGIC_BYTE;//Writes the magic value into the first byte of the heap.
        AllocatorStats* stats = (AllocatorStats*)(allocator.heapStart);// Treat this address as an AllocatorStats structure.
        stats->magicalBytes = MAGIC_BYTE;
        stats->numOfPages = 1;
        stats->numOfBlocks = 1;
        BlockHeader* firstBlock = (BlockHeader*)(allocator.heapStart + sizeof(AllocatorStats));// Treat this address as a BlockHeader structure.
        firstBlock->marker = BLOCK_MARKER;//mark the block as a valid block.
        firstBlock->inUse = false;//mark the block as free.
        firstBlock->length = length - sizeof(AllocatorStats) - sizeof(BlockHeader);//Total mem - allocator metadata -first block header
                                                                                   //gives usable memory for the user.
        firstBlock->prev = NULL;
        firstBlock->next = NULL;
        allocator.firstBlock = firstBlock;

    }



}


