#include <assert.h>
#include <malloc.h>
#include "my_malloc.h"
void insert_block(void* ptr, block* h){
    block * b = ptr - BLOCK_SIZE;
    //printf("Inserting the block of %p,%ld\n",b,b->size);
    block * pivot= h;
    if (pivot == NULL){
        h = b;
        h->prev = NULL;
        h->next = NULL;
        //printf("head is set\n");
        return;
    }
    if (b < pivot){
        //insert at front
        //printf("Insert at front\n");
        h = b;
        b->next = pivot;
        b->prev = NULL;
        pivot->prev = b;
        return;
    }
    while(pivot->next!=NULL){
        if(b > pivot && b < pivot->next){
            //insert b after pivot
            b->next = pivot->next;
            pivot->next = b;
            b->prev = pivot;
            b->next->prev = b;
            return;
        }
        pivot = pivot->next;
    }
    //printf("Insert at tail\n");
    //inserting at tail
    pivot->next = b;
    b->next = NULL;
    b->prev = pivot;
    //printf("The tail is %p",pivot->next);
}
void * allocate_block(size_t size){
    block * block = sbrk(BLOCK_SIZE + size);
    block->size = size;
    block->next = NULL;
    block->prev = NULL;
    return (void *)block + BLOCK_SIZE;
}
void * allocate_block_nolock(size_t size){
    pthread_mutex_lock(&lock);
    block * block = sbrk(BLOCK_SIZE + size);
    pthread_mutex_unlock(&lock);
    block->size = size;
    block->next = NULL;
    block->prev = NULL;
    return (void *)block + BLOCK_SIZE;
}
void * split_block(block * pivot,size_t size){
    block * used_block = (void*) pivot + BLOCK_SIZE + (pivot->size - BLOCK_SIZE -size);
    used_block->next = NULL;
    used_block->size = size;
    used_block->prev = NULL;
    //printf("The split block address:%p,size%ld\n",used_block,used_block->size);
    pivot->size = pivot->size - size - BLOCK_SIZE;
    return (void*)used_block + BLOCK_SIZE;
}
void * find_empty_block(size_t size, block * h){
    block * pivot = h;
    block * best_fit = NULL;
    int count = 0;
    while(pivot != NULL){
        count ++;
        if(pivot->size >= size && best_fit == NULL){
            best_fit = pivot;
        }else if(pivot->size >= size && pivot->size < best_fit->size){
            best_fit = pivot;
        }
        if (best_fit!= NULL && best_fit->size == size){
            break;
        }
        pivot = pivot->next;
    }
    if(best_fit == NULL){
        return NULL;
    }
    if(best_fit->size > size + BLOCK_SIZE){
        return split_block(best_fit,size);
    }else if(best_fit == h){
        h = h->next;
        if(h!=NULL) {
            h->prev = NULL;
        }
        best_fit->next = NULL;
        best_fit->prev = NULL;
        return (void*)best_fit + BLOCK_SIZE;
    }else{
        best_fit->prev->next = best_fit->next;
        if(best_fit->next!=NULL) {
            best_fit->next->prev = best_fit->prev;
        }
        return (void*)best_fit + BLOCK_SIZE;
    }
}
void merge_blocks(void * ptr) {
    block *merge_block = ptr - BLOCK_SIZE;
    while (merge_block->next != NULL && merge_block->next == ptr + merge_block->size) {
        merge_block->size += merge_block->next->size + BLOCK_SIZE;
        merge_block->next = merge_block->next->next;
        if (merge_block->next != NULL) {
            merge_block->next->prev = merge_block;
        }
    }
    while (merge_block->prev != NULL &&
           (void *) merge_block->prev + merge_block->prev->size + BLOCK_SIZE == merge_block) {
        merge_block->prev->size += merge_block->size + BLOCK_SIZE;
        merge_block->prev->next = merge_block->next;
        if (merge_block->next != NULL) {
            merge_block->next->prev = merge_block->prev;
        }
    }
}
void *ts_malloc_lock(size_t size){
    pthread_mutex_lock(&lock);
    //printf("Mallocing %ld\n",size);
    void * ptr = find_empty_block(size,head);
    void * ans;
    if(ptr != NULL){
        ans= ptr;
    }else{
        ans= allocate_block(size);
    }
    pthread_mutex_unlock(&lock);
    return ans;
}
void ts_free_lock(void * ptr){
    pthread_mutex_lock(&lock);
    insert_block(ptr,head);
    merge_blocks(ptr);
    pthread_mutex_unlock(&lock);
}
void *ts_malloc_nolock(size_t size){
    void * ptr = find_empty_block(size,head_l);
    void * ans;
    if(ptr != NULL){
        ans= ptr;
    }else{
        ans= allocate_block_nolock(size);
    }
    return ans;
}
void ts_free_nolock(void *ptr){
    insert_block(ptr,head_l);
    merge_blocks(ptr);
}
void print_helper() {
    block *pivot = head;
    if (pivot == NULL) {
        printf("NULL list");
        return;
    }
    int count = 0;
    while (pivot != NULL) {
        count++;
        printf("The %d block of %ld size, address:%p\n",count,pivot->size,pivot);
        pivot = pivot->next;
    }
    return;
}
