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

    uint8_t* heapEnd = (uint8_t*)sbrk(0);//Sbrk(0) returns the current end of the heap.
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
        
    }

    smallestBlock->inUse = true;//Mark the block as in use.
    int mustHaveSize = calculateMustHaveSize(size, smallestBlock, stats, lastBlock);//Calculate the minimum size that the block must have to fit the requested size.
    int remainingSize = mustHaveSize + 1;
    stats->numOfBlocks++;//Update the number of blocks in the allocator stats.
    void* userMemory = initializeNewBlock(smallestBlock, size, remainingSize);//Generate a new block of memory for the user.
    stats->isLocked = false;//Unlock the allocator to allow other threads to access it.
    return userMemory;//Return a pointer to the memory after the header for user use.

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
        //if(((*block)->length + sizeof(BlockHeader) >= size) && ((*block)->inUse == false))//If the block is free and can fit the requested size
        if ((*block)->length >= size + sizeof(BlockHeader) + 1 && (*block)->inUse == false)
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

BlockHeader* findLastBlock()
{
    AllocatorStats* stats = getStats();
    BlockHeader* block = (BlockHeader*)((char*)stats + sizeof(AllocatorStats));
    while(block->next != NULL)
    {
        block = block->next;//Move to the next block in the list.
    }

    return block;//Return the last block found.
}

int calculateMustHaveSize(size_t size, BlockHeader* smallestBlock, AllocatorStats* stats, BlockHeader* lastBlock)
{
    int mustHaveSize = smallestBlock->length - size - sizeof(BlockHeader);
    if(mustHaveSize <= 0)
    {
        sbrk(PAGE_SIZE);//Request one more page from the OS.
        stats->numOfPages++;//Update the number of pages in the allocator stats.
        lastBlock->length = lastBlock->length + PAGE_SIZE;//Update the length of the last block to include the new page.
        mustHaveSize = smallestBlock->length - size - sizeof(BlockHeader) -1;//Recalculate the must have size after adding a new page.
    }

    return mustHaveSize;
}

void* initializeNewBlock(BlockHeader* smallestBlock, size_t size, int remainingSize)
{
    BlockHeader* newBlock = (BlockHeader*)((char*)smallestBlock + sizeof(BlockHeader) + size);//Calculate the address of the new block by adding the size of the header and the requested size to the address of the smallest block.
    newBlock->marker = BLOCK_MARKER;//Mark the new block as valid.
    newBlock->prev = smallestBlock;
    newBlock->next = smallestBlock->next;//Set the next pointer of the new block to point to what was previously next of smallest block.
   
    if(newBlock->next != NULL)//If there is a next block after smallest block
    {
        (newBlock->next)->prev = newBlock;//Update its previous pointer to point to the new block.
    }
    smallestBlock->next = newBlock;//Update the next pointer of the smallest block to point to the new block.
    newBlock->length = remainingSize;//Mark the new block as free.
    newBlock->inUse = false;
    smallestBlock->length = size;//Update the length of the smallest block to be just enough for the requested size.
    
    return (char*)smallestBlock + sizeof(BlockHeader);//Return a pointer to the memory after the header for user use.
}

bool my_free(void* ptr)
{
    AllocatorStats* stats = getStats();
    while(stats->isLocked == true)
    {
        sleep(1);
    }

    stats->isLocked = true;
    BlockHeader* block = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));//Calculate the address of the block header by subtracting the size of the header from the user pointer.
    if(block->marker != BLOCK_MARKER)//Is this a valid block? If not, we cannot free it.
    {
        return false;//The block is not valid, so we cannot free it.
    }
    else
    {
        block->inUse = false;//Mark the block as free.
        memset(ptr, 0, block->length);
        if(block->next != NULL && (block->next)->inUse == false)//If the next block is free, merge it with the current block.
        {
            BlockHeader* notUsednextBlock = block->next;//Remembering the block that will disappear after the merge.
            if(notUsednextBlock != NULL)
            {
                block->next = notUsednextBlock->next;
            }

            if(notUsednextBlock->next != NULL)
            {
                (notUsednextBlock->next)->prev = block;
            }

            else 
            {
                block->next = NULL;
            }

            block->length = block->length + sizeof(BlockHeader) + notUsednextBlock->length;//Merge operation.
            memset((void*)notUsednextBlock, 0, sizeof(BlockHeader) + notUsednextBlock->length);
            stats->numOfBlocks--;
        }

        if(block->prev != NULL && (block->prev)->inUse == false)
        {
            BlockHeader* toDeleteBlock = block;
            block = block->prev;
            block->length += sizeof(BlockHeader) + toDeleteBlock->length;
            block->next = toDeleteBlock->next;
            if(block->next != NULL)
            {
                block->next->prev = block;
            }

            stats->numOfBlocks--;
        }

        reduceHeapSizeIfNeeded();
    }

    stats->isLocked = false;
    return true;
}

BlockHeader* findPrevUsedBlock(BlockHeader *ptr)
 {
  BlockHeader* itr = ptr;
  while (itr->prev != NULL)
   {
    itr = itr->prev;
    if (itr->inUse == true)
     {
      return itr;
     }
  }

  return NULL;
}

void reduceHeapSizeIfNeeded()
{
    BlockHeader* lastBlock = findLastBlock();
    BlockHeader* prevLastBlock = findPrevUsedBlock(lastBlock);
    if(prevLastBlock == NULL)
    {
        if(lastBlock->length > PAGE_SIZE)
        {
            lastBlock->length -= PAGE_SIZE;
        }

        prevLastBlock = lastBlock;
    }

    void *newEnd = (char*)prevLastBlock + sizeof(BlockHeader) + prevLastBlock->length;
    void *heapEnd = sbrk(0);
    while((char*)newEnd < (char*)heapEnd - PAGE_SIZE)
    {
        sbrk(-PAGE_SIZE);
        heapEnd = sbrk(0);
        AllocatorStats* stats = getStats();
        stats->numOfPages--;
    }

    if((char*)heapEnd - (char*)newEnd > sizeof(BlockHeader) + 1)
    {
        BlockHeader* newNotUsedBlock = (BlockHeader*)newEnd;
        newNotUsedBlock->marker = BLOCK_MARKER;
        newNotUsedBlock->inUse = false;
        newNotUsedBlock->prev = prevLastBlock;
        newNotUsedBlock->next = NULL;
        newNotUsedBlock->length = (char*)heapEnd - (char*)newEnd - sizeof(BlockHeader);
        prevLastBlock->next = newNotUsedBlock;
    }
}

