#ifndef WORLD_H
#define WORLD_H

#include "Chunk.h"
#include "VoxelEngine/BlockId.h"

#define WORLD_SIZE_X 100
#define WORLD_SIZE_Y 1
#define WORLD_SIZE_Z 100

void ChunkManager_init();
void ChunkManager_draw_chunks();
Chunk* ChunkManager_get_chunk(int x, int z);

BlockId world_get_block_at(int x, int y, int z);
BlockId world_destroy_block(int x, int y, int z);

bool world_place_block(int x, int y, int z, BlockId type);
void world_destroy();

#endif
