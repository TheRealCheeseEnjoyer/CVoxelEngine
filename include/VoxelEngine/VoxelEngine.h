#ifndef VOXELENGINE_H
#define VOXELENGINE_H
#include "Block.h"
#include "BlockId.h"

extern BlockData g_blockData[BLOCK_NUM_BLOCK_TYPES];

void VoxelEngine_init();
BlockId VoxelEngine_get_block_id(const char* name);
unsigned int VoxelEngine_get_atlas_id();
unsigned int VoxelEngine_get_atlas_num_textures();

#endif