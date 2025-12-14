#ifndef CHUNK_H
#define CHUNK_H

#include <cglm/mat4.h>

#include "VoxelEngine/BlockId.h"

#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 128
#define CHUNK_SIZE_Z 16
#define CHUNK_SIZE CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z

typedef struct {
    vec3 position;
    vec2 texCoords;
} Vertex;

typedef struct {
    unsigned int vbo;
    Vertex* mesh;
    unsigned int vertexCount;
} VboData;

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

    BlockId* blocks;
    VboData* vbos;
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
    BlockId* blocks;
};

void chunk_init(struct init_args* args);
void chunk_init_mesh(Chunk* chunk);
BlockId chunk_get_block(Chunk *chunk, int x, int y, int z);
void chunk_create_mesh(Chunk *chunk);
void chunk_load_all_mesh(Chunk *chunk);
void chunk_draw(Chunk *chunk);

BlockId chunk_destroy_block(Chunk *chunk, int x, int y, int z);

void chunk_set_block(Chunk *chunk, int x, int y, int z, BlockId type);

#endif
