#define _GNU_SOURCE 1
#include <unistd.h>
#include <sys/mman.h>

#include <stdio.h>
#include "myalloc.h"

void* _arena_start;
size_t adjusted_size;


int myinit(size_t size) {
    size_t page_size = getpagesize();
    if (size < MAX_ARENA_SIZE) {
        printf("Initializing arena:\n");
        printf("...requested size %ld bytes\n", size);
        printf("...pagesize is %ld bytes\n", page_size);

        printf("...adjusting size with page boundaries\n");
        adjusted_size = getpagesize();
        while (size > adjusted_size){
            adjusted_size += getpagesize();
        }
        printf("...adjusted size is %ld\n", adjusted_size);
        printf("...mapping arena with mmap()\n");
        _arena_start = mmap(NULL, adjusted_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

        printf("...arena starts at %p\n", (void*)_arena_start);
        void* current_address = _arena_start + adjusted_size;

        printf("...arena ends at %p\n", (void*)current_address);
        return adjusted_size;
    }
    return ERR_BAD_ARGUMENTS;
}
// void* myalloc(size_t size) {}

// void myfree(void* ptr) {}

int mydestroy() {
    if (_arena_start != NULL) {
        munmap(_arena_start, adjusted_size);
        _arena_start = NULL;
        return 0;
    }
    return ERR_UNINITIALIZED;
}
