#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

typedef struct _mblock_t {
    struct _mblock_t * prev;
    struct _mblock_t * next;
    size_t size;
    int status;
    void * payload;
} mblock_t;

typedef struct _mlist_t {
    mblock_t * head;
} mlist_t;

_mlist_t mlist = NULL;


int main(int argc, char * argv[]);
void *mymalloc(size_t size);
void myfree(void *ptr);


mblock_t * findLastMemlistBlock();
mblock_t * findFreeBlockOfSize(size_t size);
void splitBlockAtSize(mblock_t * block, size_t newSize);
void coallesceBlockPrev(mblock_t * freeBlock);
void coallesceBlockNext(mblock_t * freeBlock);
mblock_t * growHeapBySize(size_t size);


mblock_t * findLastMemlistBlock(){
    mblock_t * current = mlist.head;
    if (current == NULL){
        return NULL;
    }
    while(current->next != NULL){
        current = current->next;
    }
    return current;
}

mblock_t * findFreeBlockOfSize(size_t size){
    mblock_t * current = mlist.head;
    while(current != NULL){
        if (current->status == 0 && current->size >= size){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void splitBlockAtSize(mblock_t * block, size_t newSize){
    if (block->size <= newSize + MBLOCK_HEADER_SZ){
        return;
    }
    mblock_t * newBlock = (mblock_t *)((char *)block + MBLOCK_HEADER_SZ + newSize);
    newBlock->size = block->size - newSize - MBLOCK_HEADER_SZ;
    newBlock->status = 0;
    newBlock->next = block->next;
    newBlock->prev = block;
    if (block->next != NULL){
        block->next->prev = newBlock;
    }
    block->next = newBlock;
    block->size = newSize;
}

mblock_t * growHeapBySize(size_t size){
    void * mem = sbrk(size + MBLOCK_HEADER_SZ);
    if (mem == (void *)-1){
        return NULL;
    }
    mblock_t * newBlock = (mblock_t *)mem;
    newBlock->size = size;
    newBlock->status = 1;
    newBlock->next = NULL;
    newBlock->prev = findLastMemlistBlock();
    if (newBlock->prev != NULL){
        newBlock->prev->next = newBlock;
    }
    return newBlock;
}
void coallesceBlockPrev(mblock_t * freeBlock){
    if (freeBlock->prev != NULL && freeBlock->prev->status == 0){
        mblock_t * prevBlock = freeBlock->prev;
        prevBlock->size += MBLOCK_HEADER_SZ + freeBlock->size;
        prevBlock->next = freeBlock->next;

        if (freeBlock->next != NULL){
            freeBlock->next->prev = prevBlock;
        }
    }
}