#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>


// initiate global variables to track free blocks,bytes and alocated blocks,bytes
size_t freeBlocks = 0;
size_t freeBytes = 0;
size_t allocatedBlocks = 0;
size_t allocatedBytes = 0;
size_t metadataBytes = 0;

struct MallocMetaData {
    size_t size;
    bool is_free;
    MallocMetaData* next;
    MallocMetaData* prev;
};


// create a global pointer to a list that will cntain all the data sectords

MallocMetaData* head = NULL;

/*
    returns pointer to allocated memory block of at least size bytes
*/
void* smalloc(size_t size) { 
    if (size == 0 || size > 100000000) {
        return NULL;
    }
    // search for a free block with at least size bytes or allocates sbrk
    MallocMetaData* current = head;
    while (current != NULL) {
        // advance in the list until: free block with enough size if found
        if (current->is_free && current->size >= size) {
            current->is_free = false;
            // update global trackers
            freeBlocks--;
            freeBytes -= current->size;
            return (void*)(++current);
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
    // initialize the new block
    MallocMetaData* new_block = (MallocMetaData*)p;
    new_block->size = size;
    new_block->is_free = false;
    new_block->next = NULL;
    // update global trackers
    allocatedBlocks++;
    allocatedBytes += size;
    metadataBytes += sizeof(MallocMetaData);

    // update the pointer of head

    // case1: empty list:
    if (head == NULL) {
        head = new_block;
        new_block->prev = NULL;

    } else {
        // case2: non-empty list
        current->next = new_block;
        new_block->prev = current;
    }

    // return a pointer to the allocated block - excluding the metadata
    return (void*)(++new_block);
}

void* scalloc(size_t num, size_t size) {
     if (size == 0 || num == 0 || size*num > 100000000) {
        return NULL;
    }

    // search for a free block of at least num elements each size bytes that are all set to 0 or allocates sbrk
    MallocMetaData* current = head;
    while (current != NULL) {
        // advance in the list until: free block with enough size if found
        if (current->is_free && current->size >= (size * num)) {
            current->is_free = false;
            // set the bytes to zero
            current->is_free = false;
            // update global trackers
            freeBlocks--;
            freeBytes -= current->size;
            std::memset(++current , 0, num * size);

            return (void*)current;

        }
        if (current->next == NULL) {
            break;
        };
        current = current->next;
    }
    // unable to find apropiate block in the list, locate a new memory block
    MallocMetaData* new_block = (MallocMetaData*)sbrk(size*num+sizeof(MallocMetaData));
    if (new_block == (void*)-1) {
        return NULL;
    }
    // initialize the new block
    new_block->size = size*num;
    new_block->is_free = false;
    new_block->next = NULL;

    // update global trackers
    allocatedBlocks++;
    allocatedBytes += size*num;
    metadataBytes += sizeof(MallocMetaData);

    if (head == NULL) {
        head = new_block;
        new_block->prev = NULL;

    } else {
        // case2: non-empty list
        current->next = new_block;
        new_block->prev = current;
    }

    // set the bytes to zero
    std::memset(++new_block, 0, num * size);
    return (void*)(new_block);
}


void sfree(void* p) {
    if (p == NULL || (((MallocMetaData*)p - 1)->is_free)) {
        return;
    }
    // assuming that p truly points to the beginning of an allocated block
    ((MallocMetaData*)p - 1)->is_free = true;
    // update global trackers
    freeBlocks++;
    freeBytes += ((MallocMetaData*)p - 1)->size;
}

void* srealloc(void* oldp, size_t size) {
    if (size == 0 || size > 100000000) {
        return NULL;
    }
    if (oldp == NULL) {
        return smalloc(size);
    }
    // point to oldPtr BlockMetaData
    MallocMetaData* reallocBlockMetaData = ((MallocMetaData*)oldp - 1);
    if (size <= reallocBlockMetaData->size) {
        return oldp;
    }
    // find/allocate a new block with the new size
    void* newp = smalloc(size);
    // if smalloc failed, return NULL
    if (newp == NULL) {
        return NULL;
    }
    // copy the data from the old block to the new block
    std::memmove(newp, oldp, reallocBlockMetaData->size);

    // free the old block
    sfree(oldp);
    return newp;
}


size_t _num_free_blocks() {
    return freeBlocks;
}

size_t _num_free_bytes() {
    return freeBytes;
}

size_t _num_allocated_blocks() {
    return allocatedBlocks;
}

size_t _num_allocated_bytes() {
    return allocatedBytes;
}

size_t _num_meta_data_bytes() {
    return metadataBytes;
}

size_t _size_meta_data() {
    return sizeof(MallocMetaData);
}