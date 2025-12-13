#ifndef VOXELENGINE_H
#define VOXELENGINE_H
#include "Block.h"
#include "BlockId.h"

void VoxelEngine_init();
BlockData VoxelEngine_get_block_data(BlockId id);
unsigned int VoxelEngine_get_atlas_id();

#endif