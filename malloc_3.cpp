#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>

#define MAX_ORDER 10
#define MAX_BLOCK_SIZE 1024*128

// initiate global variables to track free blocks,bytes and alocated blocks,bytes
size_t freeBlocks = 0;
size_t freeBytes = 0;
size_t allocatedBlocks = 0;
size_t allocatedBytes = 0;
size_t metadataBytes = 0;


// MallocMetaData
struct MallocMetaData {
    size_t size;
    size_t usedSize;
    bool is_free;
    MallocMetaData* next;
    MallocMetaData* prev;
    void initiateFirstMetaData(size_t size, MallocMetaData* prev) {
        this->size = size;
        this->usedSize = 0;
        this->is_free = true;
        this->next = NULL;
        this->prev = prev;
    }
};


// global trackers
MallocMetaData* listOfOrders[MAX_ORDER];
bool flag = false;


void* smalloc(size_t size) {
    // first usage of smalloc, allocate the list of orders
    if(!flag) {
        for(int i = 0; i <= MAX_ORDER; i++) {
            listOfOrders[i] = NULL;
        }
        flag = true;

        // allign the initial 32 blocks of size 128 kb in memoery so their starting address is multiple of 32*128kb
        size_t allign = MAX_BLOCK_SIZE * 32;
        size_t initiatedPb = (size_t)(sbrk(0));

        // update global trackers
        allocatedBytes += ((MAX_BLOCK_SIZE*32) - (sizeof(MallocMetaData) * 32));

        // modify the heap to start from a multiply of 32*128kb
        sbrk(allign - (initiatedPb % allign));
        MallocMetaData* newBlock = (MallocMetaData*)sbrk(MAX_BLOCK_SIZE*32);

        // initiate first block
        newBlock->initiateFirstMetaData(MAX_BLOCK_SIZE-sizeof(MallocMetaData), NULL);

        //hold the prev to be the first block
        MallocMetaData* prev = newBlock;

        // point for the first block in the last cell of free arrays
        listOfOrders[MAX_ORDER] = newBlock;
        allocatedBlocks++;

        for (int i = 1; i < 32; i++) {
            // initiate newTmpBlock
            MallocMetaData* newTmpBlock = (MallocMetaData*)((size_t)newBlock + i * MAX_BLOCK_SIZE);
            newTmpBlock->initiateFirstMetaData(MAX_BLOCK_SIZE-sizeof(MallocMetaData), newTmpBlock - 1);

            // connect the newBlock to the previous block
            prev->next = newTmpBlock;
            
            // update the previous
            prev = newTmpBlock;

            //update global trackers
            allocatedBlocks++;
        }
    } else{
        // heap memory has been defined
        if (size == 0 || size > 100000000) {
            return NULL;
        }
        // allocating very large space
        if (sizeof(MallocMetaData) + size > MAX_BLOCK_SIZE) {           
            // use mmap
            MallocMetaData* ptr = (MallocMetaData*)mmap(NULL, size + sizeof(MallocMetaData), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (ptr == MAP_FAILED) {
                return NULL;
            }
            // update MetaData
            ptr->size = size;
            ptr->usedSize = size;
            ptr->is_free = false;

            // block allocated via mmap, is not connected to double linked list of regular blocks
            ptr->next = NULL;
            ptr->prev = NULL;

            // update global trackers
            allocatedBlocks++;
            allocatedBytes += size;
            return (void*)(++ptr);

        } else{
            // allocating small sized blocks
            // TODO
        }

    }



}