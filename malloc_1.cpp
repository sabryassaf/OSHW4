#include <unistd.h>
#include <stdlib.h>

void * smalloc(size_t size) {
    if (size == 0 || size > 100000000) {
        return NULL;
    }
    // try to allocate 'size' bytes. without using malloc
    void * p = sbrk(size);
    if (sbrk(0) == (void*)-1) { 
        return NULL; 
    }
    return p;
}