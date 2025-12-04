#include "World.h"

#include <stdio.h>
#include <string.h>
#include <cglm/affine.h>

#include "Chunk.h"
#include "../include/managers/ShaderManager.h"
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
    vec3 lookDir = {-view[0][2], view[1][2], -view[2][2]};
    vec3 upVector = {view[0][0], view[1][0], view[2][0]};
    vec3 rightVector = {-view[0][1], view[1][1], -view[2][1]};
    glm_vec3_normalize(lookDir);
    float halfFov = atanf(1.f / projection[1][1]);
    float aspectRatio = projection[1][1] / projection[0][0];

    vec3 topRightBoundDir = {lookDir[0], lookDir[1], lookDir[2]};
    vec3 topLeftBoundDir = {lookDir[0], lookDir[1], lookDir[2]};

    glm_vec3_rotate(topRightBoundDir, halfFov, upVector);
    glm_vec3_rotate(topLeftBoundDir, -halfFov, upVector);

    vec3 bottomRightBoundDir = {topRightBoundDir[0], topRightBoundDir[1], topRightBoundDir[2]};
    vec3 bottomLeftBoundDir = {topLeftBoundDir[0], topLeftBoundDir[1], topLeftBoundDir[2]};

    float verticalPixels = tanf( halfFov );
    float horizontalPixels = aspectRatio * verticalPixels;
    float halfHorizontalFOV = atanf( horizontalPixels );

    glm_vec3_rotate(topRightBoundDir, halfHorizontalFOV, rightVector);
    glm_vec3_rotate(bottomRightBoundDir, -halfHorizontalFOV, rightVector);
    glm_vec3_rotate(topLeftBoundDir, halfHorizontalFOV, rightVector);
    glm_vec3_rotate(bottomLeftBoundDir, -halfHorizontalFOV, rightVector);

    vec3 topRightBound = {chunkPos[0], chunkPos[1], chunkPos[2]};
    vec3 bottomRightBound = {chunkPos[0], chunkPos[1], chunkPos[2]};
    vec3 topLeftBound = {chunkPos[0], chunkPos[1], chunkPos[2]};
    vec3 bottomLeftBound = {chunkPos[0], chunkPos[1], chunkPos[2]};
    vec3 frontBound = {chunkPos[0], chunkPos[1], chunkPos[2]};
    glm_vec3_muladds(topRightBoundDir, MAX_CHUNK_DRAW_DISTANCE + 3, topRightBound);
    glm_vec3_muladds(bottomRightBoundDir, MAX_CHUNK_DRAW_DISTANCE + 3, bottomRightBound);
    glm_vec3_muladds(topLeftBoundDir, MAX_CHUNK_DRAW_DISTANCE + 3, topLeftBound);
    glm_vec3_muladds(bottomLeftBoundDir, MAX_CHUNK_DRAW_DISTANCE + 3, bottomLeftBound);
    glm_vec3_muladds(lookDir, MAX_CHUNK_DRAW_DISTANCE + 3, frontBound);

    int minX = fmin(fmin(fmin(fmin(fmin(frontBound[0], topRightBound[0]), bottomRightBound[0]), topLeftBound[0]), bottomLeftBound[0]), chunkPos[0]);
    minX = fmax(minX, 0);
    int minZ = fmin(fmin(fmin(fmin(fmin(frontBound[2], topRightBound[2]), bottomRightBound[2]), topLeftBound[2]), bottomLeftBound[2]), chunkPos[2]);
    minZ = fmax(minZ, 0);
    int maxX = fmax(fmax(fmax(fmax(fmax(frontBound[0], topRightBound[0]), bottomRightBound[0]), topLeftBound[0]), bottomLeftBound[0]), chunkPos[0]);
    maxX = fmin(maxX, WORLD_SIZE_X - 1);
    int maxZ = fmax(fmax(fmax(fmax(fmax(frontBound[2], topRightBound[2]), bottomRightBound[2]), topLeftBound[2]), bottomLeftBound[2]), chunkPos[2]);
    maxZ = fmin(maxZ, WORLD_SIZE_Z - 1);

    for (int x = minX; x <= maxX; x++ ) {
        for (int y = fmax(0, chunkPos[Y] - MAX_CHUNK_DRAW_DISTANCE); y < fmin(chunkPos[Y] + MAX_CHUNK_DRAW_DISTANCE, WORLD_SIZE_Y); y++) {
            for (int z = minZ; z <= maxZ; z++) {
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
