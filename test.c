
#include "test.h"
void debug_log(const char *msg)
{ 
    write(STDOUT_FILENO, msg, strlen(msg));
}

void basicTest()
{
    char* ptr = (char*)my_malloc(1);//Requesting 1 byte of memory from the heap.
    BlockHeader* firstBlock = (void*)ptr - sizeof(BlockHeader);//Calculating the address of the block header.
    assert(firstBlock->marker == BLOCK_MARKER);
    *ptr = 'C';
    assert(*ptr == 'C');
}

void test_bigger_than_available_malloc() {
  uint16_t* ptr = (uint16_t *)my_malloc(5000);//Requesting 5000 bytes of memory from the heap.(2 pages)
  BlockHeader* first_block = (void *)ptr - sizeof(BlockHeader);//get metadata of the first block
  for (uint16_t i = 0; i <= 2499; i = i + 1) {
    *(ptr + i) = i;
  }
  assert(first_block->marker == BLOCK_MARKER);
  assert(*ptr == 0);
  assert(*(ptr + 2) == 2);
  assert(*(ptr + 2499) == 2499);
  // little endian valid only
  assert(*((uint8_t *)ptr + 4999) == (2499 >> 8));
  assert(*((uint8_t *)ptr + 4998) == (2499 & 0xFF));
}

void test_free() {
  uint8_t* first = (uint8_t *)my_malloc(2048);
  BlockHeader* first_block = (void *)first - sizeof(BlockHeader);
  assert(first_block->next != NULL);
  assert(first_block->length == 2048);
  BlockHeader* second_block = first_block->next;
  assert(second_block->marker == BLOCK_MARKER);
  assert(second_block->inUse == false);
  assert(second_block->next == NULL);
  assert(second_block->length == PAGE_SIZE - sizeof(AllocatorStats) -
                                     (2 * sizeof(BlockHeader)) - first_block->length);
  my_free(first);
  assert(first_block->marker == BLOCK_MARKER);
  assert(first_block->next == NULL);
  assert(first_block->length == PAGE_SIZE - sizeof(AllocatorStats) - sizeof(BlockHeader));
}

void callTest(void* (testFunction)(), const char* message)
{
    pid_t pid = fork();
    if(pid == 0)
    {
        testFunction();
        exit(0);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        if(WIFSIGNALED(status))
        {
            printf("%s crashed with signal %d\n", message, WTERMSIG(status));
        }
        else
        {
            printf("%s passed\n", message);
        }
    }

}
