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

    return generateMemoryBlock(size);

}

void* generateMemoryBlock(size_t size)
{
    AllocatorStats* stats = getStats();
    while(stats->isLocked == true)//If the allocator is locked, wait until it is unlocked.
    {
        sleep(1);
    }

    stats->isLocked = true;//Lock the allocator to prevent other threads from accessing it.
    BlockHeader* block;
    BlockHeader* smallestBlock;
    BlockHeader* lastBlock;
    findSpaceAndAdd(size, &block, &smallestBlock, &lastBlock);//finds the smallest free block that can fit the requested size and adds it to the list of allocated blocks.
    if(smallestBlock == NULL)//No big enough free block was found, so we need to request more memory from the OS.
    {
        BlockHeader* lastBlock = findLastBlock();//Find the last block in the list of allocated blocks.
        while(lastBlock->length < size)//Keep requesting more memory from the OS until we have a big enough block.
        {
            sbrk(PAGE_SIZE);//Request one more page from the OS.
            lastBlock->length = lastBlock->length + PAGE_SIZE;//Update the length of the last block to include the new page.
            stats->numOfPages++;//Update the number of pages in the allocator stats.
           
        }

        smallestBlock = lastBlock;//Set the smallest block to be the new block since it is now available for allocation.
        smallestBlock->inUse = true;//Mark the block as in use.
    }

    
  
}


AllocatorStats* getStats()
{
    assert(allocator.heapStart != NULL);//Ensure that the heap has been initialized.
    AllocatorStats* stats = (AllocatorStats*)(allocator.heapStart);
    assert(stats->magicalBytes == MAGIC_BYTE);//Ensure that the magic byte is present, indicating a valid allocator.
    return stats;
}

void findSpaceAndAdd(size_t size, BlockHeader** block, BlockHeader** smallestBlock, BlockHeader** lastBlock)
{
    *block = (BlockHeader*)((char*)allocator.heapStart + sizeof(AllocatorStats));
    *smallestBlock = NULL;
    *lastBlock = *block;
    while(*block != NULL)
    {
        assert((*block)->marker == BLOCK_MARKER);//Ensure that the block is valid.
        //Might need to delete sizeof(BlockHeader) from the condition below, since the block length already includes the size of the header.
        if(((*block)->length + sizeof(BlockHeader) >= size) && ((*block)->inUse == false))//If the block is free and can fit the requested size
        {
            if(*smallestBlock == NULL || (*smallestBlock)->length > (*block)->length)//If this is the first suitable block found or it is smaller than the current smallest block
            {
                *smallestBlock = *block;//Update the smallest block found.
            }
        }

        *lastBlock = *block;//Update the last block to the current block.
        *block = (*block)->next;//Move to the next block in the list.
    }
}