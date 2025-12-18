#include "../include/Chunk.h"

#include <string.h>
#include <cglm/affine.h>
#include <glad/glad.h>

#include "ChunkManager.h"
#include "PerlinNoise.h"

#include "FaceOrientation.h"
#include "stb_image.h"
#include "Structure.h"
#include "Engine/Time.h"
#include "managers/ShaderManager.h"
#include "libs/CVector/Vector.h"
#include "VoxelEngine/Block.h"
#include "VoxelEngine/VoxelEngine.h"

#define COORDS_TO_INDEX(x, y, z) ((x) + (y) * CHUNK_SIZE_X + (z) * CHUNK_SIZE_X * CHUNK_SIZE_Y)

static Shader shader = 0;
static int atlasIndexLocation;
static int modelLocation;

int mod(int a, int b) { return (a % b + b) % b; }

BlockId height_mapper(int y) {
    if (y == 0) return BLOCK_ROCK;
    if (y > 0 && y <= 3) return BLOCK_WATER;
    if (y > 3 && y <= 5) return BLOCK_SAND;
    return BLOCK_GRASS;
}

void chunk_load_structure(Chunk *chunk, int x, int y, int z, StructureId structure) {
    StructureData data = structure_get_data(structure);
    for (int i = 0; i < data.numBlocks; i++) {
        ivec3 newBlockPos = {x + data.positions[i][0], y + data.positions[i][1], z + data.positions[i][2]};
        if (newBlockPos[0] > CHUNK_SIZE_X - 1 || newBlockPos[0] < 0 || newBlockPos[1] > CHUNK_SIZE_Y - 1 || newBlockPos[1] < 0 || newBlockPos[2] > CHUNK_SIZE_Z - 1 || newBlockPos[2] < 0)
            continue;
        chunk->blocks[COORDS_TO_INDEX(newBlockPos[0], newBlockPos[1], newBlockPos[2])] =
                data.blocks[i];
    }
}

void chunk_init(struct init_args *args) {
    args->chunk->north = ChunkManager_get_chunk(args->position[0], args->position[2] + 1);
    args->chunk->east = ChunkManager_get_chunk(args->position[0] - 1, args->position[2]);
    args->chunk->south = ChunkManager_get_chunk(args->position[0], args->position[2] - 1);
    args->chunk->west = ChunkManager_get_chunk(args->position[0] + 1, args->position[2]);

    args->chunk->vbos = vec_init_size(sizeof(*args->chunk->vbos), VoxelEngine_get_atlas_num_textures());

    memcpy(args->chunk->position, args->position, sizeof(ivec3));

    shader = sm_get_shader(SHADER_DEFAULT);
    atlasIndexLocation = glGetUniformLocation(shader, "atlasIndex");
    modelLocation = glGetUniformLocation(shader, "model");

    glm_mat4_identity(args->chunk->model);
    glm_translate(args->chunk->model, (vec3){
                      args->position[0] * CHUNK_SIZE_X, args->position[1] * CHUNK_SIZE_Y,
                      args->position[2] * CHUNK_SIZE_Z
                  });
    if (args->position[1] != 0) return;
    for (int x = 0; x < CHUNK_SIZE_X; x++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            int height = perlin_perlin2d(x + args->chunk->position[0] * CHUNK_SIZE_X,
                                         z + args->chunk->position[2] * CHUNK_SIZE_Z,
                                         0.1f, 1)
                         * 16;
            height = fmaxf(3, height);
            for (int y = 0; y <= fminf(height, CHUNK_SIZE_Y); y++) {
                if (x == 0 || z == 0 || x == CHUNK_SIZE_X - 1 || z == CHUNK_SIZE_Z - 1) {
                    args->chunk->blocks[COORDS_TO_INDEX(x, y, z)] = BLOCK_WOOD;
                    continue;
                }
                args->chunk->blocks[COORDS_TO_INDEX(x, y, z)] = height_mapper(y);
            }

            if ((float)rand() / RAND_MAX < .005f && args->chunk->blocks[COORDS_TO_INDEX(x, height, z)] == BLOCK_GRASS) {
                args->chunk->blocks[COORDS_TO_INDEX(x, height, z)] = BLOCK_DIRT;
                chunk_load_structure(args->chunk, x, height + 1, z, STRUCTURE_TREE);
            }
        }
    }
    free(args);
}

void chunk_init_mesh(Chunk *chunk) {
    chunk_create_mesh(chunk);
    chunk_load_all_mesh(chunk);
}

BlockId chunk_get_block(Chunk *chunk, int x, int y, int z) {
    if (x < 0) {
        if (!chunk->east)
            return BLOCK_INVALID_ID;
        return chunk_get_block(chunk->east, mod(x, CHUNK_SIZE_X), y, z);
    }

    if (x >= CHUNK_SIZE_X) {
        if (!chunk->west)
            return BLOCK_INVALID_ID;
        return chunk_get_block(chunk->west, mod(x, CHUNK_SIZE_X), y, z);
    }

    if (z < 0) {
        if (!chunk->south)
            return BLOCK_INVALID_ID;
        return chunk_get_block(chunk->south, x, y, mod(z, CHUNK_SIZE_Z));
    }

    if (z >= CHUNK_SIZE_Z) {
        if (!chunk->north)
            return BLOCK_INVALID_ID;
        return chunk_get_block(chunk->north, x, y, mod(z, CHUNK_SIZE_Z));
    }

    if (y < 0 || y >= CHUNK_SIZE_Y) {
        return BLOCK_INVALID_ID;
    }

    return chunk->blocks[COORDS_TO_INDEX(x, y, z)];
}

void chunk_get_surface_bounds(Chunk *chunk, ivec3 startPos, Vertex vertices[2], const FaceOrientation orientation,
                              char meshedFaces[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z]) {
    int length = 0, width = 0;
    ivec3 start = {startPos[0], startPos[1], startPos[2]};
    int startLengthDimension = 0, startWidthDimension = 0;
    int lengthLimit = 0, widthLimit = 0;
    int *lengthDimension = nullptr;
    int *widthDimension = nullptr;
    int neighborDirection;
    int neighborDirectionIndex = 0;
    int *neighbor[3];
    FaceOrientation orientationMask = FaceOrientation_to_mask(orientation);

    unsigned int currentTexture = g_blockData[chunk_get_block(chunk, start[0], start[1], start[2])].sideTextures[
        orientation];

    switch (orientation) {
        case FACE_TOP:
        case FACE_BOTTOM:
            lengthDimension = &start[0];
            widthDimension = &start[2];
            startLengthDimension = start[0];
            startWidthDimension = start[2];
            lengthLimit = CHUNK_SIZE_X;
            widthLimit = CHUNK_SIZE_Z;
            neighborDirection = start[1] + (orientation == FACE_TOP ? 1 : -1);
            neighbor[0] = lengthDimension;
            neighbor[1] = &neighborDirection;
            neighbor[2] = widthDimension;
            neighborDirectionIndex = 1;
            vertices[0].position[0] = start[0] - block_size[0] / 2;
            vertices[0].position[1] = start[1] + (orientation == FACE_TOP ? block_size[1] : -block_size[1]) / 2;
            vertices[0].position[2] = start[2] - block_size[2] / 2;
            vertices[1].position[1] = start[1] + (orientation == FACE_TOP ? block_size[1] : -block_size[1]) / 2;
            vertices[0].texCoords[0] = (orientation == FACE_TOP ? 1 : -1);
            vertices[0].texCoords[1] = 0;
            vertices[1].texCoords[0] = 0;
            vertices[1].texCoords[1] = 1;
            break;
        case FACE_LEFT:
        case FACE_RIGHT:
            lengthDimension = &start[2];
            widthDimension = &start[1];
            startLengthDimension = start[2];
            startWidthDimension = start[1];
            lengthLimit = CHUNK_SIZE_Z;
            widthLimit = CHUNK_SIZE_Y;
            neighborDirection = start[0] + (orientation == FACE_LEFT ? 1 : -1);
            neighbor[0] = &neighborDirection;
            neighbor[1] = widthDimension;
            neighbor[2] = lengthDimension;
            neighborDirectionIndex = 0;
            vertices[0].position[0] = start[0] + (orientation == FACE_LEFT ? block_size[0] : -block_size[0]) / 2;
            vertices[0].position[1] = start[1] - block_size[1] / 2;
            vertices[0].position[2] = start[2] - block_size[2] / 2;
            vertices[1].position[0] = start[0] + (orientation == FACE_LEFT ? block_size[0] : -block_size[0]) / 2;
            vertices[0].texCoords[0] = (orientation == FACE_LEFT ? 1 : -1);
            vertices[1].texCoords[0] = 0;
            vertices[0].texCoords[1] = 0;
            vertices[1].texCoords[1] = 1;
            break;
        case FACE_FRONT:
        case FACE_BACK:
            lengthDimension = &start[0];
            widthDimension = &start[1];
            startLengthDimension = start[0];
            startWidthDimension = start[1];
            lengthLimit = CHUNK_SIZE_X;
            widthLimit = CHUNK_SIZE_Y;
            neighborDirection = start[2] + (orientation == FACE_FRONT ? 1 : -1);
            neighbor[0] = lengthDimension;
            neighbor[1] = widthDimension;
            neighbor[2] = &neighborDirection;
            neighborDirectionIndex = 2;
            vertices[0].position[0] = start[0] - block_size[0] / 2;
            vertices[0].position[1] = start[1] - block_size[1] / 2;
            vertices[0].position[2] = start[2] + (orientation == FACE_FRONT ? block_size[2] : -block_size[2]) / 2;
            vertices[1].position[2] = start[2] + (orientation == FACE_FRONT ? block_size[2] : -block_size[2]) / 2;
            vertices[0].texCoords[0] = 0;
            vertices[0].texCoords[1] = 0;
            vertices[1].texCoords[0] = 1;
            vertices[1].texCoords[1] = 1;
            break;
    }

    for (*lengthDimension += 1; *lengthDimension < lengthLimit; (*lengthDimension)++) {
        BlockId block = chunk_get_block(chunk, start[0], start[1], start[2]);
        if ((meshedFaces[start[0]][start[1]][start[2]] & orientationMask) == orientationMask ||
            block == BLOCK_INVALID_ID ||
            block == BLOCK_AIR ||
            (chunk_get_block(chunk, *neighbor[0], *neighbor[1], *neighbor[2]) != BLOCK_INVALID_ID &&
             (g_blockData[chunk_get_block(chunk, *neighbor[0], *neighbor[1], *neighbor[2])].properties & PROPERTY_TRANSPARENCY) == 0) ||
             g_blockData[block].sideTextures[orientation] != currentTexture) {
            break;
        }

        length++;
    }

    for (*widthDimension += 1; *widthDimension < widthLimit; (*widthDimension)++) {
        bool isWholeLineOk = true;
        for (*lengthDimension = startLengthDimension;
             *lengthDimension <= startLengthDimension + length; (*lengthDimension)++) {
            BlockId block = chunk_get_block(chunk, start[0], start[1], start[2]);
            BlockId neighborBlock = chunk_get_block(chunk, *neighbor[0], *neighbor[1], *neighbor[2]);
            if ((meshedFaces[start[0]][start[1]][start[2]] & orientationMask) == orientationMask ||
                block == BLOCK_INVALID_ID ||
                block == BLOCK_AIR ||
                g_blockData[block].sideTextures[orientation] != currentTexture ||
                (neighborBlock != BLOCK_INVALID_ID &&
                 (g_blockData[neighborBlock].properties & PROPERTY_TRANSPARENCY) == 0)) {
                isWholeLineOk = false;
                break;
            }
        }

        if (!isWholeLineOk)
            break;

        width++;
    }

    for (*widthDimension = startWidthDimension; *widthDimension <= startWidthDimension + width; (*widthDimension)++) {
        for (*lengthDimension = startLengthDimension; *lengthDimension <= startLengthDimension + length; (*
                 lengthDimension)++) {
            meshedFaces[start[0]][start[1]][start[2]] ^= orientationMask;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (i == neighborDirectionIndex) continue;
        vertices[1].position[i] = *neighbor[i] - block_size[i] / 2;
    }

    vertices[0].texCoords[0] *= length + 1;
    vertices[0].texCoords[1] *= width + 1;
    vertices[1].texCoords[0] *= length + 1;
    vertices[1].texCoords[1] *= width + 1;
}

void chunk_update_mesh(Chunk *chunk, int targetTexture) {
    char meshedFaces[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z] = {0};

    for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
        for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
            for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                BlockId block = chunk_get_block(chunk, x, y, z);
                if (block == BLOCK_INVALID_ID || block == BLOCK_AIR)
                    continue;

                for (unsigned int i = 0; i < FACE_NUM; i++) {
                    if (chunk->vbos[g_blockData[block].sideTextures[i]].mesh == nullptr) {
                        chunk->vbos[g_blockData[block].sideTextures[i]].mesh = vec_init(sizeof(Vertex));
                    }
                }

                ivec3 blockPosition = {x, y, z};
                BlockId tempBlock = chunk_get_block(chunk, x, y + 1, z);
                Vertex vertices[2];
                if ((tempBlock == BLOCK_INVALID_ID || g_blockData[tempBlock].properties & PROPERTY_TRANSPARENCY) && (meshedFaces[x][y][z] & FACE_TOP_MASK)
                    != FACE_TOP_MASK && (targetTexture == -1 || g_blockData[block].sideTextures[FACE_TOP] == targetTexture)) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_TOP, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[0].position[0], vertices[0].position[1], vertices[1].position[2]},
                        .texCoords = {vertices[0].texCoords[0], vertices[1].texCoords[1]}
                    };
                    Vertex v2 = {
                        .position = {vertices[1].position[0], vertices[1].position[1], vertices[0].position[2]},
                        .texCoords = {vertices[1].texCoords[0], vertices[0].texCoords[1]}
                    };
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_TOP]].mesh, &vertices[0]);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_TOP]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_TOP]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_TOP]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_TOP]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_TOP]].mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x, y - 1, z);
                if ((tempBlock == BLOCK_INVALID_ID || g_blockData[tempBlock].properties & PROPERTY_TRANSPARENCY) && (
                        meshedFaces[x][y][z] & FACE_BOTTOM_MASK) !=
                    FACE_BOTTOM_MASK && (targetTexture == -1 || g_blockData[block].sideTextures[FACE_BOTTOM] == targetTexture)) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_BOTTOM, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[1].position[0], vertices[0].position[1], vertices[0].position[2]},
                        .texCoords = {vertices[1].texCoords[0], vertices[0].texCoords[1]}
                    };
                    Vertex v2 = {
                        .position = {vertices[0].position[0], vertices[0].position[1], vertices[1].position[2]},
                        .texCoords = {vertices[0].texCoords[0], vertices[1].texCoords[1]}
                    };
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BOTTOM]].mesh, &vertices[0]);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BOTTOM]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BOTTOM]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BOTTOM]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BOTTOM]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BOTTOM]].mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x + 1, y, z);
                if ((tempBlock == BLOCK_INVALID_ID || g_blockData[tempBlock].properties & PROPERTY_TRANSPARENCY) && (meshedFaces[x][y][z] & FACE_LEFT_MASK)
                    !=
                    FACE_LEFT_MASK && (targetTexture == -1 || g_blockData[block].sideTextures[FACE_LEFT] == targetTexture)) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_LEFT, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[0].position[0], vertices[1].position[1], vertices[0].position[2]},
                        .texCoords = {vertices[0].texCoords[0], vertices[1].texCoords[1]}
                    };
                    Vertex v2 = {
                        .position = {vertices[0].position[0], vertices[0].position[1], vertices[1].position[2]},
                        .texCoords = {vertices[1].texCoords[0], vertices[0].texCoords[1]}
                    };

                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_LEFT]].mesh, &vertices[0]);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_LEFT]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_LEFT]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_LEFT]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_LEFT]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_LEFT]].mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x - 1, y, z);
                if ((tempBlock == BLOCK_INVALID_ID || g_blockData[tempBlock].properties & PROPERTY_TRANSPARENCY) && (
                        meshedFaces[x][y][z] & FACE_RIGHT_MASK) !=
                    FACE_RIGHT_MASK && (targetTexture == -1 || g_blockData[block].sideTextures[FACE_RIGHT] == targetTexture)) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_RIGHT, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[0].position[0], vertices[0].position[1], vertices[1].position[2]},
                        .texCoords = {vertices[1].texCoords[0], vertices[0].texCoords[1]}
                    };
                    Vertex v2 = {
                        .position = {vertices[0].position[0], vertices[1].position[1], vertices[0].position[2]},
                        .texCoords = {vertices[0].texCoords[0], vertices[1].texCoords[1]}
                    };

                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_RIGHT]].mesh, &vertices[0]);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_RIGHT]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_RIGHT]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_RIGHT]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_RIGHT]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_RIGHT]].mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x, y, z + 1);
                if ((tempBlock == BLOCK_INVALID_ID || g_blockData[tempBlock].properties & PROPERTY_TRANSPARENCY) && (
                        meshedFaces[x][y][z] & FACE_FRONT_MASK) !=
                    FACE_FRONT_MASK && (targetTexture == -1 || g_blockData[block].sideTextures[FACE_FRONT] == targetTexture)) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_FRONT, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[1].position[0], vertices[0].position[1], vertices[0].position[2]},
                        .texCoords = {vertices[1].texCoords[0], vertices[0].texCoords[1]}
                    };
                    Vertex v2 = {
                        .position = {vertices[0].position[0], vertices[1].position[1], vertices[0].position[2]},
                        .texCoords = {vertices[0].texCoords[0], vertices[1].texCoords[1]}
                    };

                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_FRONT]].mesh, &vertices[0]);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_FRONT]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_FRONT]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_FRONT]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_FRONT]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_FRONT]].mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x, y, z - 1);
                if ((tempBlock == BLOCK_INVALID_ID || g_blockData[tempBlock].properties & PROPERTY_TRANSPARENCY) && (meshedFaces[x][y][z] & FACE_BACK_MASK)
                    !=
                    FACE_BACK_MASK && (targetTexture == -1 || g_blockData[block].sideTextures[FACE_BACK] == targetTexture)) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_BACK, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[0].position[0], vertices[1].position[1], vertices[0].position[2]},
                        .texCoords = {vertices[0].texCoords[0], vertices[1].texCoords[1]}
                    };
                    Vertex v2 = {
                        .position = {vertices[1].position[0], vertices[0].position[1], vertices[0].position[2]},
                        .texCoords = {vertices[1].texCoords[0], vertices[0].texCoords[1]}
                    };

                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BACK]].mesh, &vertices[0]);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BACK]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BACK]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BACK]].mesh, &v2);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BACK]].mesh, &v1);
                    vec_append(&chunk->vbos[g_blockData[block].sideTextures[FACE_BACK]].mesh, &vertices[1]);
                }
            }
        }
    }
}

void chunk_create_mesh(Chunk *chunk) {
    chunk_update_mesh(chunk, -1);
}

void chunk_load_all_mesh(Chunk *chunk) {
    if (chunk->VAO == 0)
        glGenVertexArrays(1, &chunk->VAO);

    glBindVertexArray(chunk->VAO);

    for (size_t i = 0; i < vec_size(chunk->vbos); i++) {
        if (chunk->vbos[i].mesh == nullptr) continue;
        glGenBuffers(1, &chunk->vbos[i].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->vbos[i].vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (vec_size(chunk->vbos[i].mesh) * sizeof(Vertex)),
                     chunk->vbos[i].mesh, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void chunk_load_mesh(Chunk *chunk, unsigned int type) {
    glBindVertexArray(chunk->VAO);
    if (chunk->vbos[type].vbo == 0)
        glGenBuffers(1, &chunk->vbos[type].vbo);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbos[type].vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (vec_size(chunk->vbos[type].mesh) * sizeof(Vertex)),
                 chunk->vbos[type].mesh, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void chunk_draw(Chunk *chunk) {
    glBindVertexArray(chunk->VAO);
    glActiveTexture(GL_TEXTURE0);

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, chunk->model[0]);
    chunk->lastDrawnFrame = Time.frameNumber;

    for (size_t i = 0; i < vec_size(chunk->vbos); i++) {
        if (chunk->vbos[i].vbo == 0 || vec_size(chunk->vbos[i].mesh) == 0) continue;
        glBindBuffer(GL_ARRAY_BUFFER, chunk->vbos[i].vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
        glUniform1i(atlasIndexLocation, i);
        glDrawArrays(GL_TRIANGLES, 0, (int) vec_size(chunk->vbos[i].mesh));
    }
}

void chunk_block_updated_at(Chunk *chunk, int x, int y, int z) {
    BlockId block = chunk_get_block(chunk, x, y, z);
    if (block == BLOCK_INVALID_ID || block == BLOCK_AIR) return;

    int uniqueTextures[FACE_NUM] = {-1, -1, -1, -1, -1, -1};
    int uniqueTexturesNum = 0;

    for (int i = 0; i < FACE_NUM; i++) {
        bool found = false;
        for (int j = 0; j <= uniqueTexturesNum; j++) {
            if (uniqueTextures[j] == g_blockData[block].sideTextures[i]) {
                found = true;
                break;
            }
        }

        if (!found) {
            uniqueTextures[uniqueTexturesNum] = g_blockData[block].sideTextures[i];
            uniqueTexturesNum++;
        }
    }

    for (int i = 0; i < uniqueTexturesNum; i++) {
        if (chunk->vbos[uniqueTextures[i]].mesh != nullptr)
            vec_clear(chunk->vbos[uniqueTextures[i]].mesh);
        chunk_update_mesh(chunk, uniqueTextures[i]);
        chunk_load_mesh(chunk, uniqueTextures[i]);
    }
}

void chunk_register_changes(Chunk *chunk, int x, int y, int z, BlockId changedBlockId) {
    unsigned int neighborTextures[FACE_NUM * 2] = {0}; // 6 faces for the block + 1 face for each adjacent block
    int lastIndex = 0;

    for (int i = 0; i < FACE_NUM; i++) {
        bool isAlreadyInArray = false;
        for (int j = 0; j < lastIndex; j++) {
            if (neighborTextures[j] == g_blockData[changedBlockId].sideTextures[i]) {
                isAlreadyInArray = true;
                break;
            }
        }

        if (!isAlreadyInArray) {
            neighborTextures[lastIndex++] = g_blockData[changedBlockId].sideTextures[i];
        }

        unsigned int neighborTexture = 0;;
        FaceOrientation neighborFaceOrientation = 0;
        BlockId neighborBlock = BLOCK_INVALID_ID;
        switch (i) {
            case FACE_TOP:
                neighborBlock = chunk_get_block(chunk, x, y + 1, z);
                neighborFaceOrientation = FACE_BOTTOM;
                break;
            case FACE_BOTTOM:
                neighborBlock = chunk_get_block(chunk, x, y - 1, z);
                neighborFaceOrientation = FACE_TOP;
                break;
            case FACE_LEFT:
                neighborBlock = chunk_get_block(chunk, x + 1, y, z);
                neighborFaceOrientation = FACE_RIGHT;
                break;
            case FACE_RIGHT:
                neighborBlock = chunk_get_block(chunk, x - 1, y, z);
                neighborFaceOrientation = FACE_LEFT;
                break;
            case FACE_FRONT:
                neighborBlock = chunk_get_block(chunk, x, y, z + 1);
                neighborFaceOrientation = FACE_BACK;
                break;
            case FACE_BACK:
                neighborBlock = chunk_get_block(chunk, x, y, z - 1);
                neighborFaceOrientation = FACE_FRONT;
                break;
        }

        if (neighborBlock == BLOCK_INVALID_ID || neighborBlock == BLOCK_AIR)
            continue;

        neighborTexture = g_blockData[neighborBlock].sideTextures[neighborFaceOrientation];

        isAlreadyInArray = false;
        for (int j = 0; j < lastIndex; j++) {
            if (neighborTextures[j] == neighborTexture) {
                isAlreadyInArray = true;
                break;
            }
        }

        if (!isAlreadyInArray) {
            neighborTextures[lastIndex++] = neighborTexture;
        }
    }

    for (int i = 0; i < lastIndex; i++) {
        unsigned int toUpdate = neighborTextures[i];
        if (chunk->vbos[toUpdate].mesh != nullptr)
            vec_clear(chunk->vbos[toUpdate].mesh);
        chunk_update_mesh(chunk, toUpdate);
        chunk_load_mesh(chunk, toUpdate);
    }

    if (x == CHUNK_SIZE_X - 1 && chunk->west != nullptr) {
        chunk_block_updated_at(chunk->west, 0, y, z);
    } else if (x == 0 && chunk->east != nullptr) {
        chunk_block_updated_at(chunk->east, CHUNK_SIZE_X - 1, y, z);
    }

    if (z == 0 && chunk->south != nullptr) {
        chunk_block_updated_at(chunk->south, x, y, CHUNK_SIZE_Z - 1);
    } else if (z == CHUNK_SIZE_Z - 1 && chunk->north != nullptr) {
        chunk_block_updated_at(chunk->north, x, y, 0);
    }
}

BlockId chunk_destroy_block(Chunk *chunk, int x, int y, int z) {
    BlockId oldType = chunk_get_block(chunk, x, y, z);
    chunk_set_block(chunk, x, y, z, BLOCK_AIR);

    chunk_register_changes(chunk, x, y, z, oldType);
    return oldType;
}

void chunk_set_block(Chunk *chunk, int x, int y, int z, BlockId type) {
    chunk->blocks[COORDS_TO_INDEX(x, y, z)] = type;
    chunk_register_changes(chunk, x, y, z, type);
}

bool chunk_is_already_drawn(Chunk* chunk) {
    return Time.frameNumber == chunk->lastDrawnFrame;
}
