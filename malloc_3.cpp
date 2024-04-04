#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>

#define MAX_ORDER 10

// initiate global variables to track free blocks,bytes and alocated blocks,bytes
size_t freeBlocks = 0;
size_t freeBytes = 0;
size_t allocatedBlocks = 0;
size_t allocatedBytes = 0;
size_t metadataBytes = 0;


// MallocMetaData
struct MallocMetaData {
    size_t size;
    bool is_free;
    MallocMetaData* next;
    MallocMetaData* prev;
};

// global trackers
MallocMetaData* listOfOrders[MAX_ORDER];
bool flag = false;

void* smalloc(size_t size) {
    // first usage of smalloc, allocate the list of orders
    if(!flag) {
        flag = true;
        for(int i = 0; i < MAX_ORDER; i++) {
            listOfOrders[i] = NULL;
        }
    // alligh the initial 32 blocks of size 128 kb in memoery so their starting address is multiple of 32*128kb
        

    }


}