#ifndef STACK_H
#define STACK_H
#include "VoxelEngine/BlockId.h"

typedef struct {
    BlockId type;
    int size;
    int maxSize;
} BlockStack;

#define STACK_EMPTY (BlockStack){0, 0, 0}

#endif
