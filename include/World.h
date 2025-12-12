#ifndef WORLD_H
#define WORLD_H

#include "VoxelEngine/Block.h"
#include "VoxelEngine/BlockId.h"

#define WORLD_SIZE_X 100
#define WORLD_SIZE_Y 1
#define WORLD_SIZE_Z 100

void world_init(void);
void world_draw(void);
BlockId world_get_block_at(int x, int y, int z);

BlockId world_destroy_block(int x, int y, int z);

bool world_place_block(int x, int y, int z, BlockId type);
void world_destroy();

#endif
