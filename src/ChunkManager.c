#include "ChunkManager.h"

#include <string.h>
#include <GLFW/glfw3.h>

#include "Chunk.h"
#include "managers/ShaderManager.h"
#include "Player.h"
#include "thpool.h"
#include "VoxelEngine/Block.h"

#define CHUNK_COORDS_TO_INDEX(x, z) ((x) + (z) * WORLD_SIZE_X)
#define GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z) (CHUNK_COORDS_TO_INDEX((x) / CHUNK_SIZE_X, (z) / CHUNK_SIZE_Z))
#define WORLD_NUM_CHUNKS WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z
#define MAX_CHUNK_DRAW_DISTANCE 12

static Chunk chunks[WORLD_NUM_CHUNKS];

Chunk* ChunkManager_get_chunk(int x, int z) {
    if (x < 0 || x >= WORLD_SIZE_X || z < 0 || z >= WORLD_SIZE_Z) return nullptr;
    return &chunks[CHUNK_COORDS_TO_INDEX(x, z)];
}

void ChunkManager_init() {
    for (int z = 0; z < WORLD_SIZE_Z; z++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int x = 0; x < WORLD_SIZE_Z; x++) {
                ivec3 position = {x, y, z};
                struct init_args* args = malloc(sizeof(struct init_args));
                    args->chunk = ChunkManager_get_chunk(x, z);
                    memcpy(args->position, position, sizeof(ivec3));
                //thpool_add_work((void*)chunk_init, args);
                chunk_init(args);
            }
        }
    }

    // Wait since we need all chunks to be ready to be meshed for the next step
    thpool_wait();

    for (int z = 0; z < WORLD_SIZE_X; z++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int x = 0; x < WORLD_SIZE_Z; x++) {
                //chunk_create_mesh(get_chunk(x, y, z));
                thpool_add_work((void*)chunk_create_mesh, ChunkManager_get_chunk(x, z));
            }
        }
    }

    // Wait since we need to do the mesh loading on the main thread due to OpenGL context not working in worker threads
    thpool_wait();

    for (int z = 0; z < WORLD_SIZE_X; z++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int x = 0; x < WORLD_SIZE_Z; x++) {
                chunk_load_all_mesh(ChunkManager_get_chunk(x, z));
            }
        }
    }

    thpool_destroy();
}

void ChunkManager_draw_chunks() {
    shader_use(sm_get_shader(SHADER_DEFAULT));
    shader_set_int(sm_get_shader(SHADER_DEFAULT), "TextureUnitId", 0);
    vec3 playerPos;
    mat4 projection;
    mat4 view;
    player_get_position(playerPos);
    player_get_projection(projection);
    player_get_view(view);

    shader_set_mat4(sm_get_shader(SHADER_DEFAULT), "projection", &projection);
    shader_set_mat4(sm_get_shader(SHADER_DEFAULT), "view", &view);

    vec3 chunkPos = {playerPos[0] / CHUNK_SIZE_X, playerPos[1] / CHUNK_SIZE_Y, playerPos[2] / CHUNK_SIZE_Z};
    vec3 lookDir = {-view[0][2], view[1][2], -view[2][2]};
    vec3 planarLookDir = {lookDir[0], 0, lookDir[2]};
    vec3 rightVector = {view[0][0], view[1][0], view[2][0]};
    vec3 upVector = {-view[0][1], view[1][1], -view[2][1]};
    glm_vec3_normalize(lookDir);
    glm_vec3_normalize(planarLookDir);
    float halfFov = atanf(1.f / projection[1][1]);
    float aspect = projection[1][1] / projection[0][0];
    float verticalPixels = tanf(halfFov);
    float horizontalPixels = aspect * verticalPixels;
    float halfHorizontalFOV = atanf(horizontalPixels);

    vec3 boundsDir[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            boundsDir[i][j] = lookDir[j];
        }
    }

    glm_vec3_rotate(boundsDir[0], halfFov, upVector);
    glm_vec3_rotate(boundsDir[0], halfHorizontalFOV, rightVector);
    glm_vec3_rotate(boundsDir[1], halfFov, upVector);
    glm_vec3_rotate(boundsDir[1], -halfHorizontalFOV, rightVector);
    glm_vec3_rotate(boundsDir[2], -halfFov, upVector);
    glm_vec3_rotate(boundsDir[2], halfHorizontalFOV, rightVector);
    glm_vec3_rotate(boundsDir[3], -halfFov, upVector);
    glm_vec3_rotate(boundsDir[3], -halfHorizontalFOV, rightVector);

    vec3 bounds[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            bounds[i][j] = chunkPos[j];
        }
    }

    for (int i = 0; i < 4; i++) {
        glm_vec3_muladds(boundsDir[i], MAX_CHUNK_DRAW_DISTANCE + 1, bounds[i]);
    }

    vec3 frontBound = {chunkPos[0], chunkPos[1], chunkPos[2]};
    if (planarLookDir[2] >= fabs(planarLookDir[0])) {
        glm_vec3_muladds((vec3){0, 0, 1}, MAX_CHUNK_DRAW_DISTANCE, frontBound);
    } else if (planarLookDir[2] <= -fabs(planarLookDir[0])) {
        glm_vec3_muladds((vec3){0, 0, -1}, MAX_CHUNK_DRAW_DISTANCE, frontBound);
    } else if (planarLookDir[0] >= fabs(planarLookDir[2])) {
        glm_vec3_muladds((vec3){1, 0, 0}, MAX_CHUNK_DRAW_DISTANCE, frontBound);
    } else if (planarLookDir[0] <= -fabs(planarLookDir[2])) {
        glm_vec3_muladds((vec3){-1, 0, 0}, MAX_CHUNK_DRAW_DISTANCE, frontBound);
    }

    int minX = fmin(fmin(fmin(fmin(fmin(chunkPos[0], frontBound[0]), bounds[0][0]), bounds[1][0]), bounds[2][0]), bounds[3][0]) - 1;
    int maxX = fmax(fmax(fmax(fmax(fmax(chunkPos[0], frontBound[0]), bounds[0][0]), bounds[1][0]), bounds[2][0]), bounds[3][0]) + 1;

    minX = fmax(minX, 0);
    maxX = fmin(maxX, WORLD_SIZE_X - 1);

    int minZ = fmin(fmin(fmin(fmin(fmin(chunkPos[2], frontBound[2]), bounds[0][2]), bounds[1][2]), bounds[2][2]),bounds[3][2]) - 1;
    int maxZ = fmax(fmax(fmax(fmax(fmax(chunkPos[2], frontBound[2]), bounds[0][2]), bounds[1][2]), bounds[2][2]),bounds[3][2]) + 1;

    minZ = fmax(minZ, 0);
    maxZ = fmin(maxZ, WORLD_SIZE_Z - 1);

    glDisable(GL_BLEND);
    for (int x = minX; x <= maxX; x++ ) {
        for (int z = minZ; z <= maxZ; z++) {
            if (glm_vec3_distance2(chunkPos, (vec3){x, 0, z}) < MAX_CHUNK_DRAW_DISTANCE * MAX_CHUNK_DRAW_DISTANCE)
                chunk_draw(ChunkManager_get_chunk(x, z));
        }
    }
    glEnable(GL_BLEND);

    shader_use(0);
}

BlockId world_get_block_at(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X * CHUNK_SIZE_X || y < 0 || y >= WORLD_SIZE_Y * CHUNK_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z * CHUNK_SIZE_Z)
        return BLOCK_INVALID_ID;

    return chunk_get_block(&chunks[GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z)], x % CHUNK_SIZE_X, y % CHUNK_SIZE_Y, z % CHUNK_SIZE_Z);
}

BlockId world_destroy_block(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X * CHUNK_SIZE_X || y < 0 || y >= WORLD_SIZE_Y * CHUNK_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z * CHUNK_SIZE_Z)
        return 0;

    return chunk_destroy_block(&chunks[GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z)], x % CHUNK_SIZE_X, y % CHUNK_SIZE_Y, z % CHUNK_SIZE_Z);
}

bool world_place_block(int x, int y, int z, BlockId type) {
    if (x < 0 || x >= WORLD_SIZE_X * CHUNK_SIZE_X || y < 0 || y >= WORLD_SIZE_Y * CHUNK_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z * CHUNK_SIZE_Z)
        return false;

    chunk_set_block(&chunks[GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z)], x % CHUNK_SIZE_X, y % CHUNK_SIZE_Y, z % CHUNK_SIZE_Z, type);
    return true;
}
