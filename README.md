# Custom Memory Allocator in C

## Overview
A custom implementation of dynamic memory allocation, similar to malloc/free.
The allocator manages its own heap, maintains metadata for allocated blocks,
splits free blocks, merges adjacent free blocks, and requests additional memory
pages from the operating system when needed.

## Features
- Custom malloc implementation
- Custom free implementation
- Block metadata management
- Free block reuse
- Block splitting
- Adjacent block merging
- Heap expansion using sbrk()
- Allocation statistics tracking
- Memory corruption detection using block markers


## How It Works

The allocator manages memory using a linked list of blocks.
Each block stores:
- Size of the memory region
- Whether it is allocated or free
- Previous and next block pointers
- Validation marker

## Example Usage

void* ptr = my_malloc(100);

int* array = (int*)ptr;

array[0] = 10;

my_free(ptr);

## Testing 
The project includes tests for:
- Basic allocation
- Requesting additional memory pages
- Basic freeing
- Block splitting
- Block merging

Each test is implemented as a separate function and executed in its own process using `fork()`.

Running each test in a child process provides two important benefits:

- If a test triggers an `assert()` or crashes due to a memory error, only the child process terminates.
- The remaining tests continue to execute, allowing the test suite to report multiple failures in a single run instead of stopping at the first error.
### Example output:
Basic Malloc passed

Request more memory Malloc passed

Basic Free passed

