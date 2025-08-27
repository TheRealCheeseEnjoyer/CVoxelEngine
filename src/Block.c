#include "Block.h"
#include "Constants.h"

void block_get_aabb(vec3 pos, AABB *out) {
    out->max[X] = pos[X] + block_size[X] / 2;
    out->max[Y] = pos[Y] + block_size[Y] / 2;
    out->max[Z] = pos[Z] + block_size[Z] / 2;
    out->min[X] = pos[X] - block_size[X] / 2;
    out->min[Y] = pos[Y] - block_size[Y] / 2;
    out->min[Z] = pos[Z] - block_size[Z] / 2;
}
