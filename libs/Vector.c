#include "Vector.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DEFAULT_CAPACITY 16
#define DEFAULT_MULTIPLIER 2

typedef struct {
    size_t itemSize;
    size_t size;
    size_t capacity;
} head;

void* vec_init(size_t itemSize) {
    return vec_init_cap(itemSize, DEFAULT_CAPACITY);
}

void* vec_init_cap(size_t itemSize, size_t capacity) {
    head* memory = malloc(sizeof(head) + itemSize * capacity);
    if (memory == NULL) {
        return NULL;
    }

    memory->itemSize = itemSize;
    memory->size = 0;
    memory->capacity = DEFAULT_CAPACITY;
    return memory + 1;
}

int vec_append(void* vec, void* item) {
    void** v = vec;
    head* h = *v - sizeof(head);
    if (h->size + 1 > h->capacity) {
        size_t newCapacity = h->capacity * DEFAULT_MULTIPLIER;
        head* newMemory = realloc(h, sizeof(head) + newCapacity * h->itemSize);
        if (newMemory == NULL) {
            printf("vec_append: realloc failed\n");
            return 0;
        }
        h = newMemory;
        h->capacity = newCapacity;
        *v = h + 1;
    }
    memcpy(*v + h->itemSize * h->size, item, h->itemSize);
    h->size += 1;
    return 1;
}

size_t vec_capacity(void *vec) {
    head* h = vec - sizeof(head);
    return h->capacity;
}

size_t vec_size(void *vec) {
    head* h = vec - sizeof(head);
    return h->size;
}

void vec_clear(void *vec) {
    head* h = vec - sizeof(head);
    h->size = 0;
}

void vec_free(void* memory) {
    head* h = memory - sizeof(head);
    free(h);
}