#include "../include/VoxelEngine/Block.h"

void block_get_aabb(vec3 pos, AABB *out) {
    out->center[0] = pos[0];
    out->center[1] = pos[1];
    out->center[2] = pos[2];
    out->extent[0] = block_size[0] / 2;
    out->extent[1] = block_size[1] / 2;
    out->extent[2] = block_size[2] / 2;
}