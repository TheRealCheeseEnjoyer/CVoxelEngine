#include "World.h"

#include <string.h>

#include "Chunk.h"
#include "ShaderManager.h"
#include "Constants.h"
#include "thpool.h"

#define CHUNK_COORDS_TO_INDEX(x, y, z) (x + y * WORLD_SIZE_X + z * WORLD_SIZE_X * WORLD_SIZE_Y)
#define GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z) (CHUNK_COORDS_TO_INDEX(x / CHUNK_SIZE_X, y / CHUNK_SIZE_Y, z / CHUNK_SIZE_Z))
#define WORLD_NUM_CHUNKS WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z
#define WORLD_SIZE WORLD_NUM_CHUNKS * CHUNK_SIZE
#define MAX_CHUNK_DRAW_DISTANCE 12

Chunk* chunks;
Block* blocks;

Chunk* get_chunk(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X || y < 0 || y >= WORLD_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z) return nullptr;
    return &chunks[CHUNK_COORDS_TO_INDEX(x, y, z)];
}

void world_init(vec3 initialPosition) {
    blocks = calloc(CHUNK_SIZE * WORLD_NUM_CHUNKS, sizeof(Block));
    chunks = calloc(WORLD_NUM_CHUNKS, sizeof(Chunk));

    for (int z = 0; z < WORLD_SIZE_Z; z++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int x = 0; x < WORLD_SIZE_Z; x++) {
                ivec3 position = {x, y, z};
                struct init_args* args = malloc(sizeof(struct init_args));
                    args->chunk = get_chunk(x, y, z);
                    memcpy(args->position, position, sizeof(ivec3));
                    args->north = get_chunk(x, y, z + 1);
                    args->south = get_chunk(x, y, z - 1);
                    args->east = get_chunk(x - 1, y, z);
                    args->west = get_chunk(x + 1, y, z);
                    args->above = get_chunk(x, y + 1, z);
                    args->below = get_chunk(x, y - 1, z);
                    args->blocks = &blocks[CHUNK_COORDS_TO_INDEX(x, y, z) * CHUNK_SIZE];
                thpool_add_work((void*)chunk_init, args);
                //chunk_init(args);
            }
        }
    }

    // Wait since we need all chunks to be ready to be meshed for the next step
    thpool_wait();

    for (int z = 0; z < WORLD_SIZE_X; z++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int x = 0; x < WORLD_SIZE_Z; x++) {
                thpool_add_work((void*)chunk_create_mesh, get_chunk(x, y, z));
            }
        }
    }

    // Wait since we need to do the mesh loading on the main thread due to OpenGL context not working in worker threads
    thpool_wait();

    for (int z = 0; z < WORLD_SIZE_X; z++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int x = 0; x < WORLD_SIZE_Z; x++) {
                chunk_load_mesh(get_chunk(x, y, z));
            }
        }
    }
}

void world_draw(vec3 playerPos, mat4 projection, mat4 view) {
    shader_use(sm_get_shader(SHADER_DEFAULT));
    vec3 chunkPos = {playerPos[X] / CHUNK_SIZE_X, playerPos[Y] / CHUNK_SIZE_Y, playerPos[Z] / CHUNK_SIZE_Z};

    for (int x = fmax(0, chunkPos[X] - MAX_CHUNK_DRAW_DISTANCE); x < fmin(chunkPos[X] + MAX_CHUNK_DRAW_DISTANCE, WORLD_SIZE_X); x++ ) {
        for (int y = fmax(0, chunkPos[Y] - MAX_CHUNK_DRAW_DISTANCE); y < fmin(chunkPos[Y] + MAX_CHUNK_DRAW_DISTANCE, WORLD_SIZE_Y); y++) {
            for (int z = fmax(0, chunkPos[Z] - MAX_CHUNK_DRAW_DISTANCE); z < fmin(chunkPos[Z] + MAX_CHUNK_DRAW_DISTANCE, WORLD_SIZE_Z); z++) {
                if (glm_vec3_distance2(chunkPos, (vec3){x, y, z}) < MAX_CHUNK_DRAW_DISTANCE * MAX_CHUNK_DRAW_DISTANCE)
                    chunk_draw(get_chunk(x, y, z), projection, view);
            }
        }
    }

    shader_use(0);
}

Block * world_get_block_at(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X * CHUNK_SIZE_X || y < 0 || y >= WORLD_SIZE_Y * CHUNK_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z * CHUNK_SIZE_Z)
        return nullptr;

    return chunk_get_block(&chunks[GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z)], x % CHUNK_SIZE_X, y % CHUNK_SIZE_Y, z % CHUNK_SIZE_Z);
}

BlockType world_destroy_block(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X * CHUNK_SIZE_X || y < 0 || y >= WORLD_SIZE_Y * CHUNK_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z * CHUNK_SIZE_Z)
        return 0;

    return chunk_destroy_block(&chunks[GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z)], x % CHUNK_SIZE_X, y % CHUNK_SIZE_Y, z % CHUNK_SIZE_Z);
}

bool world_place_block(int x, int y, int z, BlockType type) {
    if (x < 0 || x >= WORLD_SIZE_X * CHUNK_SIZE_X || y < 0 || y >= WORLD_SIZE_Y * CHUNK_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z * CHUNK_SIZE_Z)
        return false;

    return chunk_place_block(&chunks[GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z)], x % CHUNK_SIZE_X, y % CHUNK_SIZE_Y, z % CHUNK_SIZE_Z, type);
}
