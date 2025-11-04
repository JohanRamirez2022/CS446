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

//comment out the line below if you are compiling on Windows
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

mlist_t mlist = {NULL};


int main(int argc, char * argv[]);
void *mymalloc(size_t size);
void myfree(void *ptr);

void printMemList(const mblock_t* head);
mblock_t * findLastMemlistBlock();
mblock_t * findFreeBlockOfSize(size_t size);
void splitBlockAtSize(mblock_t * block, size_t newSize);
void coallesceBlockPrev(mblock_t * freeBlock);
void coallesceBlockNext(mblock_t * freeBlock);
mblock_t * growHeapBySize(size_t size);

int main(int argc, char * argv[]){
    void * p1 = mymalloc(10);
    void * p2 = mymalloc(100);
    void * p3 = mymalloc(200);
    void * p4 = mymalloc(500);
    myfree(p3); p3 = NULL;
    myfree(p2); p2 = NULL;
    void * p5 = mymalloc(150);
    void * p6 = mymalloc(500);
    myfree(p4); p4 = NULL;
    myfree(p5); p5 = NULL;
    myfree(p6); p6 = NULL;
    myfree(p1); p1 = NULL;
}

void *mymalloc(size_t size){
    mblock_t * firstBlock = findFreeBlockOfSize(size);
    if (firstBlock != NULL){
        splitBlockAtSize(firstBlock, size);
        firstBlock->status = 1;
    }
    else{
        mblock_t * newBlock = growHeapBySize(size);
        if (newBlock == NULL){
            return NULL;
        }
        if (mlist.head == NULL){
            mlist.head = newBlock;
        }
        firstBlock = newBlock;
    }
    return (void *)((char *)firstBlock + MBLOCK_HEADER_SZ);

}

void myfree(void *ptr){
    if (ptr == NULL){
        return;
    }
    mblock_t * blockToFree = (mblock_t *)((char *)ptr - MBLOCK_HEADER_SZ);
    blockToFree->status = 0;
    coallesceBlockPrev(blockToFree);
    coallesceBlockNext(blockToFree);
}


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
    if (block->size <= newSize + MBLOCK_HEADER_SZ + 1){
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


void coallesceBlockNext(mblock_t * freedBlock){
    if (freedBlock->next != NULL && freedBlock->status == 0){
        mblock_t * nextBlock = freedBlock->next;
        freedBlock->size += MBLOCK_HEADER_SZ + nextBlock->size;
        freedBlock->next = nextBlock->next;
        if (nextBlock->next != NULL){
            nextBlock->next->prev = freedBlock;
        }
    }
}

void printMemList(const mblock_t* head) {
    const mblock_t* p = head;
    size_t i = 0;
    while(p != NULL) {
        printf("[%ld] p: %p\n", i, (void*)p);
        printf("[%ld] p->size: %ld\n", i, p->size);
        printf("[%ld] p->status: %s\n", i, p->status > 0 ? "allocated" : "free");
        printf("[%ld] p->prev: %p\n", i, (void*)p->prev);
        printf("[%ld] p->next: %p\n", i, (void*)p->next);
        printf("___________________________\n");
        ++i;
        p = p->next;
    }
    printf("===========================\n");
}
