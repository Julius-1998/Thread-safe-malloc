
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct _block{
    size_t size;
    struct _block *next;
    struct _block *prev;
} block;

block *head = NULL;
__thread block *head_l = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

const size_t BLOCK_SIZE = sizeof(block);

void *ts_malloc_lock(size_t size);
void ts_free_lock(void * ptr);

void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void * ptr);

void print_helper();