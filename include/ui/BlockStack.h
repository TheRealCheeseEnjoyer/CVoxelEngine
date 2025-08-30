#ifndef STACK_H
#define STACK_H
#include "BlockType.h"

typedef struct {
    BlockType type;
    int size;
    int maxSize;
} BlockStack;

#define STACK_EMPTY (BlockStack){0, 0, 0}

#endif
