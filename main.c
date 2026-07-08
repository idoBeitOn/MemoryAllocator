/*
Goals:
- Keeps a list of memory locations inside the heap where the memory is free.
- Keeps track of all allocations made.
- On demand, it can return a new free memory area of a given size.
   If the size is bigger than all existing free blocks, 
   request the OS to grow the heap.
- On demand, it can free a chosen block.
   Freeing blocks can result in a request to the OS to reduce the size of the heap.
- It is thread-safe (it can be used from multiple threads at the same time.)
*/


