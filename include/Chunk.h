#ifndef CHUNK_H
#define CHUNK_H

#include "Block.h"
#include "../libs/Vector.h"
#include <cglm/mat4.h>

#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 16
#define CHUNK_SIZE_Z 16
#define CHUNK_SIZE CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z

typedef struct {
    vec3 position;
    vec2 texCoords;
} Vertex;

typedef struct chunk_t {
    ivec3 position;
    mat4 model;
    unsigned int VAO;
    struct chunk_t *north;
    struct chunk_t *south;
    struct chunk_t *east;
    struct chunk_t *west;
    struct chunk_t *above;
    struct chunk_t *below;

    Block *blocks;
    unsigned int vbos[BLOCK_NUM_BLOCK_TYPES]; // Maps BlockType to vbo
    Vector meshes[BLOCK_NUM_BLOCK_TYPES]; // Maps BlockType to mesh
} Chunk;

struct init_args {
    Chunk* chunk;
    ivec3 position;
    Chunk* north;
    Chunk* south;
    Chunk* east;
    Chunk* west;
    Chunk* above;
    Chunk* below;
    Block* blocks;
};

void chunk_init(struct init_args* args);
void chunk_init_mesh(Chunk* chunk);
Block *chunk_get_block(Chunk *chunk, int x, int y, int z);
void chunk_create_mesh(Chunk *chunk);
void chunk_load_mesh(Chunk *chunk);
void chunk_draw(Chunk *chunk, mat4 projection, mat4 view);

BlockType chunk_destroy_block(Chunk *chunk, int x, int y, int z);

bool chunk_place_block(Chunk *chunk, int x, int y, int z, BlockType type);

#endif
