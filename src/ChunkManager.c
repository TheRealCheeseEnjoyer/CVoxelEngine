#include "ChunkManager.h"

#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>

#include "Chunk.h"
#include "managers/ShaderManager.h"
#include "Player.h"
#include "thpool.h"
#include "Engine/Time.h"
#include "VoxelEngine/Block.h"
#include "VoxelEngine/VoxelEngine.h"
#include "Geometry.h"

#define CHUNK_COORDS_TO_INDEX(x, z) ((x) + (z) * WORLD_SIZE_X)
#define GLOBAL_COORDS_TO_CHUNK_INDEX(x, y, z) (CHUNK_COORDS_TO_INDEX((x) / CHUNK_SIZE_X, (z) / CHUNK_SIZE_Z))
#define WORLD_NUM_CHUNKS WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z
#define MAX_CHUNK_DRAW_DISTANCE 12

typedef struct {
    Chunk* arr[64];
    int back;
    int front;
} Queue;

static Chunk chunks[WORLD_NUM_CHUNKS];

void enqueue(Queue* q, Chunk* chunk) {
    q->arr[q->back] = chunk;
    q->back = (q->back + 1) % 64;
}

Chunk* dequeue(Queue* q) {
    Chunk* chunk = q->arr[q->front];
    q->front = (q->front + 1) % 64;
    return chunk;
}

bool is_empty(Queue* q) {
    return q->back == q->front;
}

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
    vec3 rightVector = {view[0][0], view[1][0], view[2][0]};
    vec3 upVector = {-view[0][1], view[1][1], -view[2][1]};
    glm_vec3_normalize(lookDir);
    float halfFov = atanf(1.f / projection[1][1]);
    float aspect = projection[1][1] / projection[0][0];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, VoxelEngine_get_atlas_id());

    float zNear = projection[3][2] / (projection[2][2] - 1);
    float zFar = projection[3][2] / (projection[2][2] + 1);
    float halfVSide = zFar * tanf(halfFov);
    float halfHSide = halfVSide * aspect;
    vec3 eyePos;
    player_get_eye_position(eyePos);
    vec3 frontMulFar;
    glm_vec3_scale(lookDir, MAX_CHUNK_DRAW_DISTANCE,frontMulFar);
    Plane rightPlane;
    Plane leftPlane;

    vec3 normal;
    vec3 temp;
    glm_vec3_scale(rightVector, halfHSide, temp);
    glm_vec3_sub(frontMulFar, temp, temp);
    glm_vec3_cross(temp, upVector, normal);
    glm_normalize_to(normal, rightPlane.normal);
    rightPlane.distance = glm_dot(rightPlane.normal, eyePos);

    glm_vec3_scale(rightVector, halfHSide, temp);
    glm_vec3_add(frontMulFar, temp, temp);
    glm_vec3_cross(upVector, temp, normal);
    glm_normalize_to(normal, leftPlane.normal);
    leftPlane.distance = glm_dot(leftPlane.normal, eyePos);

    AABB chunkAABB;
    chunkAABB.extent[0] = CHUNK_SIZE_X / 2;
    chunkAABB.extent[1] = CHUNK_SIZE_Y * 2;
    chunkAABB.extent[2] = CHUNK_SIZE_Z / 2;

    Queue queue = {};
    enqueue(&queue, ChunkManager_get_chunk(chunkPos[0], chunkPos[2]));

    glDisable(GL_BLEND);
    while (!is_empty(&queue)) {
        Chunk* chunk = dequeue(&queue);
        chunkAABB.center[0] = chunk->position[0] * CHUNK_SIZE_X + chunkAABB.extent[0];
        chunkAABB.center[1] = 0 + chunkAABB.extent[1];
        chunkAABB.center[2] = chunk->position[2] * CHUNK_SIZE_Z + chunkAABB.extent[2];
        float rDist = chunkAABB.extent[0] * fabs(rightPlane.normal[0]) + chunkAABB.extent[1] * fabs(rightPlane.normal[1]) + chunkAABB.extent[2] * fabs(rightPlane.normal[2]);
        float lDist = chunkAABB.extent[0] * fabs(leftPlane.normal[0]) + chunkAABB.extent[1] * fabs(leftPlane.normal[1]) + chunkAABB.extent[2] * fabs(leftPlane.normal[2]);
        if (-rDist <= glm_dot(rightPlane.normal, chunkAABB.center) - rightPlane.distance && -lDist <= glm_dot(leftPlane.normal, chunkAABB.center) - leftPlane.distance) {
            if (glm_vec3_distance2(chunkPos, (vec3){chunk->position[0], chunk->position[1], chunk->position[2]}) < MAX_CHUNK_DRAW_DISTANCE * MAX_CHUNK_DRAW_DISTANCE) {
                chunk_draw(chunk);
                if (chunk->north != nullptr && !chunk_is_already_drawn(chunk->north)) {
                    enqueue(&queue, chunk->north);
                    chunk->north->lastDrawnFrame = Time.frameNumber;
                }
                if (chunk->south != nullptr && !chunk_is_already_drawn(chunk->south)) {
                    enqueue(&queue, chunk->south);
                    chunk->south->lastDrawnFrame = Time.frameNumber;
                }
                if (chunk->east != nullptr && !chunk_is_already_drawn(chunk->east)) {
                    enqueue(&queue, chunk->east);
                    chunk->east->lastDrawnFrame = Time.frameNumber;
                }
                if (chunk->west != nullptr && !chunk_is_already_drawn(chunk->west)) {
                    enqueue(&queue, chunk->west);
                    chunk->west->lastDrawnFrame = Time.frameNumber;
                }
            }
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
