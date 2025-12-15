#ifndef CVOXELENGINE_STRUCTUREID_H
#define CVOXELENGINE_STRUCTUREID_H
#include <cglm/vec3.h>

#include "VoxelEngine/BlockId.h"

typedef enum {
    STRUCTURE_TREE,
    STRUCTURE_NUM,
    STRUCTURE_INVALID_ID
} StructureId;

typedef struct {
    unsigned int numBlocks;
    ivec3* positions;
    BlockId* blocks;
} StructureData;

void structure_init();
StructureData structure_get_data(StructureId id);

#endif