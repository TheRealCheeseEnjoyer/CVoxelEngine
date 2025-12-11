#include "Block.h"

void block_get_aabb(vec3 pos, AABB *out) {
    out->max[0] = pos[0] + block_size[0] / 2;
    out->max[1] = pos[1] + block_size[1] / 2;
    out->max[2] = pos[2] + block_size[2] / 2;
    out->min[0] = pos[0] - block_size[0] / 2;
    out->min[1] = pos[1] - block_size[1] / 2;
    out->min[2] = pos[2] - block_size[2] / 2;
}
