#include "../include/World.h"
#include "../include/Chunk.h"

#define CHUNK_COORDS_TO_INDEX(x, y, z) (x + y * WORLD_SIZE_X + z * WORLD_SIZE_X * WORLD_SIZE_Y)
#define WORLD_NUM_CHUNKS WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z
#define WORLD_SIZE WORLD_NUM_CHUNKS * CHUNK_SIZE
Chunk* chunks;
Block* blocks;
Shader shader;

Chunk* get_chunk(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X || y < 0 || y >= WORLD_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z) return nullptr;
    return &chunks[CHUNK_COORDS_TO_INDEX(x, y, z)];
}

void world_init(vec3 initialPosition) {
    blocks = calloc(CHUNK_SIZE * WORLD_NUM_CHUNKS, sizeof(Block));
    chunks = calloc(WORLD_NUM_CHUNKS, sizeof(Chunk));

    shader = shader_create("shaders/default.vsh", "shaders/default.fsh");

    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
                ivec3 position = {x, y, z};
                chunk_init(
                    get_chunk(x, y, z),
                    position,
                    get_chunk(x, y, z + 1),
                    get_chunk(x, y, z - 1),
                    get_chunk(x - 1, y, z),
                    get_chunk(x + 1, y, z),
                    &blocks[CHUNK_COORDS_TO_INDEX(x, y, z) * CHUNK_SIZE]
                    );
            }
        }
    }

    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
                chunk_create_mesh(get_chunk(x, y, z));
                chunk_load_mesh(get_chunk(x, y, z));
            }
        }
    }
}

void world_draw(mat4 projection, mat4 view) {
    shader_use(shader);
    for (int i = 0; i < WORLD_NUM_CHUNKS; i++) {
        chunk_draw(&chunks[i], shader, projection, view);
    }
    shader_use(0);
}
