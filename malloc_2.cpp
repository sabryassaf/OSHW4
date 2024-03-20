#include <unistd.h>
#include <stdlib.h>

struct MallocMetaData {
    size_t size;
    bool is_free;
    MallocMetaData* next;
    MallocMetaData* prev;
};


//create a global pointer to a list that will cntain all the data sectords

MallocMetaData* head = NULL;

void* smalloc(size_t size) {
    if (size == 0 || size > 100000000) {
        return NULL;
    }
    //search for a free block with at least size bytes or allocates sbrk
    MallocMetaData* current = head;
    while (current != NULL) {
        //advance in the list until: free block with enough size if found
        if (current->is_free && current->size >= size) {
            current->is_free = false;
            return (void*)(current + sizeof(MallocMetaData));
        }
        if (current->next == NULL) {
            break;
        };
        current = current->next;
    }

    // unable to find apropiate block in the list, locate a new memory block
    void* p = sbrk(size+sizeof(MallocMetaData));
    if (sbrk(0) == (void*)-1) { 
        return NULL; 
    }
    //initialize the new block
    MallocMetaData* new_block = (MallocMetaData*)p;
    new_block->size = size;
    new_block->is_free = false;
    new_block->next = NULL;
    new_block->prev = current;

    //return a pointer to the allocated block - excluding the metadata
    return (void*)(new_block + sizeof(MallocMetaData));
};

void* scalloc(size_t num, size_t suze) {
    
}

