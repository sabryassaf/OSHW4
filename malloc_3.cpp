#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>

#define MAX_ORDER 10
#define MAX_BLOCK_SIZE 128*1024

// initiate global variables to track free blocks,bytes and alocated blocks,bytes
size_t allocatedBlocks = 0;
size_t allocatedBytes = 0;

bool status = false;

// MallocMetaData
struct MallocMetaData {
    size_t size;
    bool is_free;
    MallocMetaData* next;
    MallocMetaData* prev;
    void initiateFirstMetaData(size_t size, MallocMetaData* prev) {
        this->size = size;
        this->is_free = true;
        this->next = NULL;
        this->prev = prev;
    }
};

size_t _size_meta_data() {
    return sizeof(MallocMetaData);
}

size_t _num_allocated_blocks() {
    return allocatedBlocks;
}



// global trackers
MallocMetaData* listOfOrders[MAX_ORDER];

size_t _num_free_blocks() {
    int num = 0;
    for (int i = 0; i <= MAX_ORDER; i++) {
        MallocMetaData* tmp = listOfOrders[i];
        while (tmp != NULL) {
            num++;
            tmp = tmp->next;
        }
    }
    return num;
}


int findRealOrderOfBlock(size_t size) {
    int order = 0;
    // include metaDataSize
    size += sizeof(MallocMetaData);
    while (size > 128) {
        size = size >> 1;
        order++;
    }
    return order;
}
/*
remove block from list without changing its availability status
*/
void removeBlockFromOrderList(MallocMetaData* block) {
    int order = findRealOrderOfBlock(block->size);
    // if its the first block
    if (block->prev == NULL) {
        listOfOrders[order] = block->next;
    } else {
        block->prev->next = block->next;
    }
    if (block->next != NULL) {
        block->next->prev = block->prev;
    }
    // update the block
    block->next = NULL;
    block->prev = NULL;
    block->is_free = false;
}

void addBlockToOrderList(MallocMetaData* block) {
    size_t order = findRealOrderOfBlock(block->size);
    MallocMetaData* currentElement = listOfOrders[order];
    if (listOfOrders[order] == NULL) {
        listOfOrders[order] = block;
        block->next = NULL;
        block->prev = NULL;
        return;
    } else {
        // added in ascedning order of the sizes in the current listofrders[order]
        if ((size_t)block < (size_t)currentElement) {
            block->next = currentElement;
            block->prev = NULL;
            currentElement->prev = block;
            listOfOrders[order] = block;
            return;
        } else {
            while ((size_t) block > (size_t) currentElement) {
                if (currentElement->next == NULL) {
                    currentElement->next = block;
                    block ->prev = currentElement;
                    block ->next = NULL;
                    return;
                }
                currentElement = currentElement->next;
            }
            //insert the node
                MallocMetaData* previous = currentElement->prev;
                previous->next = block;
                block ->prev = previous;
                block->next = currentElement;
                currentElement->prev = block;
                return;
        }
    }
}

/*
@param: size, block
@return: split the block recursively until the block is of the required size
*/
void splitBlock(int size, MallocMetaData* block) {
    // check if the current block has double the size of used block + 2 metadata and bigger than 128 byte
    while ((block->size+ sizeof(MallocMetaData)) / 2 > (size + sizeof(MallocMetaData)) && ((block->size + sizeof(MallocMetaData) / 2) > 128)) {
        // split the block, initiate a new block
        MallocMetaData* newBlock = (MallocMetaData*)((size_t)block + (block->size + sizeof(MallocMetaData))/ 2);
        newBlock->size = (block->size - sizeof(MallocMetaData)) / 2;
        removeBlockFromOrderList(block);
        newBlock->is_free = true;
        newBlock->prev = block;
        // remove old block from the list of orders
        block->size = (block->size - sizeof(MallocMetaData)) / 2;
        block->is_free = true;

        // add the both blocks to the list of orders
        addBlockToOrderList(block);
        addBlockToOrderList(newBlock);
        // update global trackers
        allocatedBlocks++;
        // we have added an extra meta data
        allocatedBytes -= sizeof(MallocMetaData);
    }
}

/*
find the best available block in the list of orders
return -1 if not found
*/
int findOrderOfBlock(size_t size) {
    int order = findRealOrderOfBlock(size);
    for (int i = order; i <= MAX_ORDER; i++) {
        if (listOfOrders[i] != NULL) {
            return i;
        }
    }
    // no suitable order found
    return -1;
}

/*
allocate tightest fitting block
split incase needed
*/
void* allcoateBlock(size_t size) {
    int order = findOrderOfBlock(size);
    if (order == -1) {
        return NULL;
    }
    // we always use the first block in the list of orders[order]
    // check if possible split
    splitBlock(size, listOfOrders[order]);

    // check if there is better orderOfBlock after splitting
    order = findOrderOfBlock(size);
    if (order == -1) {
        return NULL;
    }

    MallocMetaData* block = listOfOrders[order];
    block->is_free = false;
    removeBlockFromOrderList(block);
    return (void*)(++block);
}

MallocMetaData* locateBuddy(MallocMetaData* block) {
    if ( block == NULL) {
        return NULL;
    }
    if (block->size > MAX_BLOCK_SIZE) {
        return NULL;
    }
    size_t blockSize = block->size + sizeof(MallocMetaData);
    //XOR between block address and size
    MallocMetaData* buddy = (MallocMetaData*)((size_t)block ^ blockSize);
    return buddy;
}

/*
merge free blocks in the list of orders
*/
// void mergeFreeBlocksAUX(MallocMetaData* block) {

//     if ((block->size + sizeof(MallocMetaData)) >= MAX_BLOCK_SIZE/2) {
//         return;
//     }
//     // find a buddy to merge
//     MallocMetaData* buddy = locateBuddy(block);
//     if (buddy == NULL) {
//         return;
//     }
//     // check if buddy is free
//     if (buddy->is_free && (buddy->size == block->size)) {
//         // remove buddy from the list of orders
//         removeBlockFromOrderList(buddy);
//         // merge the two blocks
//         MallocMetaData* newBlock = (MallocMetaData*)((size_t)block < (size_t)buddy ? block : buddy);
//         newBlock->size = block->size * 2 + sizeof(MallocMetaData);
//         newBlock->is_free = true;
//         newBlock->next = NULL;
//         newBlock->prev = NULL;
//         removeBlockFromOrderList(block);
//         // add the new block to the list of orders
//         addBlockToOrderList(newBlock);

//         // remove block from list of orders
//         block->is_free = true;

//         // update global trackers
//         allocatedBlocks--;
//         std::cout<<"i am merging for size"<<block->size<<std::endl;
//         allocatedBytes += sizeof(MallocMetaData);
//         // recursively merge the new block
//         mergeFreeBlocksAUX(newBlock);
//     }
// }
void mergeWithBuddy(MallocMetaData* block, MallocMetaData* buddy) {
    // merge block with buddy by erasing buddy's metadata and updating block's metadata to refer
    // to the new merged block
    if (block > buddy) {
        MallocMetaData* tmp = block;
        block = buddy;
        buddy = tmp;
    }

    // remove buddy from the list of orders
    removeBlockFromOrderList(buddy);
    removeBlockFromOrderList(block);
    // merge the two blocks
    block->size = block->size * 2 + sizeof(MallocMetaData);
    block->is_free = true;
    block->next = NULL;
    block->prev = NULL;
    // add the new block to the list of orders
    addBlockToOrderList(block);
    // update global trackers
    --allocatedBlocks;
    allocatedBytes += sizeof(MallocMetaData);
}

void mergeFreeBlocks(MallocMetaData* block) {
    if (findRealOrderOfBlock(block->size) == MAX_ORDER) {
        return;
    }
    // check if there is a buddy
    MallocMetaData* buddy = locateBuddy(block);
    if (buddy == NULL) {
        return;
    }
    if (block->size != buddy->size) {
        addBlockToOrderList(block);
        return;
    }
    if (buddy->is_free) {
        mergeWithBuddy(block,buddy);
        mergeFreeBlocks(block);
    } else { 
        addBlockToOrderList(block);
    }
}

// define a global flag

void* smalloc(size_t size) {
    // first usage of smalloc, allocate the list of orders
    if(!status) {
        for(int i = 0; i <= MAX_ORDER; i++) {
            listOfOrders[i] = NULL;
        }
        //update global flag to true
        status = true;
        // allign the initial 32 blocks of size 128 kb in memoery so their starting address is multiple of 32*128kb
        size_t allign = MAX_BLOCK_SIZE * 32;
        size_t initiatedPb = (size_t) sbrk(0);

        // update global trackers
        allocatedBytes += (MAX_BLOCK_SIZE*32) - (32 * sizeof(MallocMetaData));

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
            newTmpBlock->initiateFirstMetaData(MAX_BLOCK_SIZE-sizeof(MallocMetaData), prev);
            prev->next = (MallocMetaData *)((size_t)newBlock + i * MAX_BLOCK_SIZE);
            prev = prev->next;

            //update global trackers
            allocatedBlocks++;
        }
    } 
    // heap memory has been defined
    if (size == 0 || size > 100000000) {
        return NULL;
    }
    // allocating very large space
    if ((sizeof(MallocMetaData) + size) > (MAX_BLOCK_SIZE)) {           
        // use mmap
        MallocMetaData* ptr = (MallocMetaData*)mmap(NULL, size + sizeof(MallocMetaData), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            return NULL;
        }
        // update MetaData
        ptr->size = size;
        ptr->is_free = false;

        // block allocated via mmap, is not connected to double linked list of regular blocks
        ptr->next = NULL;
        ptr->prev = NULL;

        // update global trackers
        allocatedBlocks++;
        allocatedBytes += size;

        return (void*)(++ptr);

    } else {
        void* tmp =  allcoateBlock(size);
        return tmp;
    }
}


void* scalloc(size_t number, size_t size) {
    // allocate requested size
    void* block = smalloc(number * size);

    // test if failed
    if (block == NULL) {
        return NULL;
    }
    return memset(block, 0, number * size);
}

void sfree(void* p) {
    // test if valid
    if (p == NULL) {
        return;
    }
    // point to MetaData
    MallocMetaData* blockToFree = (MallocMetaData*)(p);
    --blockToFree;
    if(blockToFree->is_free) {
        return;
    }
    // check if block is allocated via mmap
    if ((blockToFree->size + sizeof(MallocMetaData)) > MAX_BLOCK_SIZE) {
        // save tmp data before freeing
        blockToFree->is_free = true;
        size_t tmpSize = blockToFree->size;
        //free the block
        munmap((void*) blockToFree, blockToFree->size + sizeof(MallocMetaData));
        // update global trackers
        allocatedBlocks--;
        allocatedBytes -= tmpSize;
        return;
    } 
    // update block status
    blockToFree->is_free = true;
    // merge free blocks
    if (findRealOrderOfBlock(blockToFree->size) == MAX_ORDER) {
        addBlockToOrderList(blockToFree);
        return;
    }
    mergeFreeBlocks(blockToFree);
}

void* srealloc(void* oldp, size_t size) {
    return NULL;    
}

size_t _num_allocated_bytes() {
    return allocatedBytes;
}

size_t _num_free_bytes() {
    int num = 0;
    for (int i = 0; i <= MAX_ORDER; i++) {
        MallocMetaData* tmp = listOfOrders[i];
        while (tmp != NULL) {
            num += tmp->size;
            tmp = tmp->next;
        }
    }
    return num;
}

size_t _num_meta_data_bytes() {
    return _size_meta_data() * _num_allocated_blocks();
}