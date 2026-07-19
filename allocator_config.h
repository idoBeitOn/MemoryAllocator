#ifndef ALLOCATOR_CONFIG_H
#define ALLOCATOR_CONFIG_H


#include <stdint.h>
#include "BlockHeader.h"

/*
The allocator's configuration/constants file.
It defines values that the allocator logic will use everywhere.
*/

#define MAGIC_BYTE 0x55 //for debugging purposes.
#define BLOCK_MARKER 0xDD
#define FIRST_BLOCK_OFFSET sizeof(BlockHeader)
#define PAGE_SIZE 4096U

#endif