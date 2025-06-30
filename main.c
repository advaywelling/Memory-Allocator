#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "main.h"

pthread_mutex_t mem_lock;

typedef union header alloc_header;

alloc_header *head = NULL, *tail = NULL; // pointers to starting and ending block of memory

alloc_header *get_free_block(size_t req_size) {
    alloc_header *curr = head;
    while(curr) {
        if(curr->mem.is_free && curr->mem.size >= req_size) {
            return curr + 1;
        }
        curr = curr->mem.next;
    }
    return NULL;
}

void* mem_alloc(size_t size) {
    if (!size) {
        return NULL;
    }
    pthread_mutex_lock(&mem_lock);
    // find free block from current chain
    alloc_header *new_mem = get_free_block(size);
    if (new_mem) {
        new_mem->mem.is_free = 0;
        pthread_mutex_unlock(&mem_lock);
        return (void*)(new_mem);
    }
    // otherwise ask OS for memory
    size_t allocate_size = sizeof(alloc_header) + size;
    void* block = sbrk(allocate_size);
    if (block == (void*)-1) {
        pthread_mutex_unlock(&mem_lock);
        return NULL;
    }
    new_mem = block;
    new_mem->mem.size = size;
    new_mem->mem.is_free = 0;
    new_mem->mem.next = NULL;
    if (!head) {
        head = new_mem;
    }
    else {
        tail->mem.next = new_mem;
        tail = new_mem;
    }
    pthread_mutex_unlock(&mem_lock);
    return (void*)(new_mem + 1);
}

void* cont_alloc(size_t count, size_t size) {
    if (!count || !size) {
        return NULL;
    }
    void* block;
    size_t alloc_size;
    alloc_size = count * size;
    if (size != alloc_size / count) {
        return NULL;
    }
    block = mem_alloc(alloc_size);
    if (!block) {
        return NULL;
    }
    memset(block, 0, alloc_size);
    return block;
}

void* re_alloc(void* block, size_t size) {
    if (!block || !size) {
        return NULL;
    }
    alloc_header* curr_mem = (alloc_header*)block - 1;
    if(curr_mem->mem.size >= size) {
        return block;
    }
    void* realloc_mem = mem_alloc(size);
    if (realloc_mem) {
        memcpy(realloc_mem, block, size);
        free_mem(block);
    }
    return realloc_mem;
}

void free_mem(void* block) {
    if (!block) {
        return;
    }
    pthread_mutex_lock(&mem_lock);
    alloc_header* mem_block = (alloc_header*)block - 1;
    void* curr_heap = sbrk(0); // end of heap
    if ((mem_block->mem.size + (char*)block) == curr_heap) {
        if (head == tail) {
            head = tail = NULL;
        } else {
            alloc_header* temp_ptr = head;
            while (temp_ptr) {
                if (temp_ptr->mem.next == tail) {
                    temp_ptr->mem.next = NULL;
                    tail = temp_ptr;
                    break;
                }
                temp_ptr = temp_ptr->mem.next;
            }
        }
        sbrk(0 - (sizeof(alloc_header) + mem_block->mem.size));
    } else {
        mem_block->mem.is_free = 1;
    }
    pthread_mutex_unlock(&mem_lock);
}

int main(void) {
    return 0;
}
