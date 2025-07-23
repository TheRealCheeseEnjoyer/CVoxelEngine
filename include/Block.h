#ifndef VOXEL_H
#define VOXEL_H

#include "AABB.h"
#include "BlockType.h"

constexpr vec3 block_size = {1, 1, 1};

typedef struct {
    BlockType type;
} Block;

void block_get_aabb(vec3 pos, AABB* out);

#endif
