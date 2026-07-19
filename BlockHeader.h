#ifndef BLOCKHEADER_H
#define BLOCKHEADER_H


#include <stdint.h>
#include <stdbool.h>

/*
Defines metadata for one block of memory.
Every block begins with a header.
The user receives a pointer to the memory after the header.
*/
typedef struct BlockHeader
{
    uint8_t marker; 
    uint32_t length;//Stores the size of the user's memory
    bool inUse;
    struct BlockHeader* prev;
    struct BlockHeader* next;
} BlockHeader;


#endif BLOCKHEADER_H