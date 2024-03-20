#include <unistd.h>
#include <stdlib.h>

void * smalloc(size_t size) {
    // try to allocate 'size' bytes
    void *p = malloc(size);
    if (p == NULL) {
        if (size == 0) { return NULL;}
        //if size > 10^8 return NULL
        else if (size > 100000000) { return NULL; }
        //if sbrk fails return NULL
        
    } else { return p; }
}