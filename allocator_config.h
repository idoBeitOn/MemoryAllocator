#include <stdint.h>
#include "BlockHeader.h"

/*
The allocator's configuration/constants file.
It defines values that the allocator logic will use everywhere.
*/

#define MAGIC_BYTE 0x55 //for debugging purposes.
#define BLOCK_MARKER 0xDD
const int FIRST_BLOCK_OFFSET = sizeof(BlockHeader);//To skip the metadata when returning a pointer to the user.
const int PAGE_SIZE = 4096;