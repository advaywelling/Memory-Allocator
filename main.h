#include "stdio.h"

union  header {
    struct {
        size_t size; // size of memory block
        unsigned is_free; // status of block
        union header *next; // pointer to next block
    } mem;
    char align[16]; // 16 byte aligned
};

void* mem_alloc(size_t size);
union header *get_free_block(size_t req_size);
void* cont_alloc(size_t count, size_t size);
void* re_alloc(void* block, size_t size);
void free_mem(void *block);
