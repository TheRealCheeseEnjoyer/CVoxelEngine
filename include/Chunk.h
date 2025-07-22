#ifndef CHUNK_H
#define CHUNK_H

#include <cglm/mat4.h>
#include "Block.h"
#include "Shader.h"
#include "Vector.h"

#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 16
#define CHUNK_SIZE_Z 16
#define CHUNK_SIZE CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z

typedef struct {
    vec3 position;
    vec2 texCoords;
} Vertex;

typedef struct chunk_t {
    vec3 position;
    mat4 model;
    unsigned int VAO;
    struct chunk_t *north;
    struct chunk_t *south;
    struct chunk_t *east;
    struct chunk_t *west;

    Block *blocks;
    unsigned int vbos[BLOCK_NUM_BLOCK_TYPES]; // Maps BlockType to vbo
    Vector meshes[BLOCK_NUM_BLOCK_TYPES]; // Maps BlockType to mesh
} Chunk;

void chunk_init(Chunk *chunk, ivec3 position, Chunk *north, Chunk *south, Chunk *east, Chunk *west,
                Block *blocks);
Block *chunk_get_block(Chunk *chunk, int x, int y, int z);
void chunk_create_mesh(Chunk *chunk);
void chunk_load_mesh(Chunk *chunk);
void chunk_draw(Chunk *chunk, Shader shader, mat4 projection, mat4 view);

void chunk_destroy_block(Chunk* chunk, int x, int y, int z);
void chunk_place_block(Chunk* chunk, int x, int y, int z, BlockType type);

#endif
