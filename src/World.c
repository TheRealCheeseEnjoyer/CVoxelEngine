#include "../include/World.h"
#include "../include/Chunk.h"
#include "../include/ShaderManager.h"
#include "../include/Constants.h"

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
                chunk_init(
                    get_chunk(x, y, z),
                    position,
                    get_chunk(x, y, z + 1),
                    get_chunk(x, y, z - 1),
                    get_chunk(x - 1, y, z),
                    get_chunk(x + 1, y, z),
                    get_chunk(x, y + 1, z),
                    get_chunk(x, y - 1, z),
                    &blocks[CHUNK_COORDS_TO_INDEX(x, y, z) * CHUNK_SIZE]
                    );
            }
        }
    }

    for (int z = 0; z < WORLD_SIZE_X; z++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int x = 0; x < WORLD_SIZE_Z; x++) {
                chunk_create_mesh(get_chunk(x, y, z));
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

void world_destroy_block(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X * CHUNK_SIZE_X || y < 0 || y >= WORLD_SIZE_Y * CHUNK_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z * CHUNK_SIZE_Z)
        return;

    chunk_destroy_block(&chunks[GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z)], x % CHUNK_SIZE_X, y % CHUNK_SIZE_Y, z % CHUNK_SIZE_Z);
}

void world_place_block(int x, int y, int z, BlockType type) {
    if (x < 0 || x >= WORLD_SIZE_X * CHUNK_SIZE_X || y < 0 || y >= WORLD_SIZE_Y * CHUNK_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z * CHUNK_SIZE_Z)
        return;

    chunk_place_block(&chunks[GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z)], x % CHUNK_SIZE_X, y % CHUNK_SIZE_Y, z % CHUNK_SIZE_Z, type);
}
