#ifndef WORLD_H
#define WORLD_H

#include "Block.h"
#include "Shader.h"

#define WORLD_SIZE_X 200
#define WORLD_SIZE_Y 1
#define WORLD_SIZE_Z 200

void world_init(void);
void world_draw(vec3 playerPos, mat4 projection, mat4 view);
Block* world_get_block_at(int x, int y, int z);

BlockType world_destroy_block(int x, int y, int z);

bool world_place_block(int x, int y, int z, BlockType type);
void world_destroy();

#endif
