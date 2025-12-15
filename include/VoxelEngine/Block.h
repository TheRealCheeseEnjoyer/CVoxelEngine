#ifndef VOXEL_H
#define VOXEL_H

#include "AABB.h"
#include "FaceOrientation.h"
#include "BlockProperty.h"

constexpr vec3 block_size = {1, 1, 1};
static constexpr int MaxNameLen = 12;

typedef struct {
    char name[MaxNameLen];
    unsigned int sideTextures[FACE_NUM];
    BlockProperty properties;
} BlockData;

void block_get_aabb(vec3 pos, AABB* out);

#endif
