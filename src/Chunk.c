#include "../include/Chunk.h"

#include <iso646.h>
#include <string.h>
#include <cglm/affine.h>
#include <glad/glad.h>

#include "PerlinNoise.h"

#include "FaceOrientation.h"
#include "stb_image.h"
#include "managers/ShaderManager.h"
#include "libs/CVector/Vector.h"
#include "VoxelEngine/Block.h"
#include "VoxelEngine/VoxelEngine.h"

#define COORDS_TO_INDEX(x, y, z) (x + y * CHUNK_SIZE_X + z * CHUNK_SIZE_X * CHUNK_SIZE_Y)

Shader shader = 0;

int mod(int a, int b) { return (a % b + b) % b; }

BlockId height_mapper(int y) {
    if (y == 0) return BLOCK_ROCK;
    if (y > 0 && y <= 3) return BLOCK_WATER;
    if (y > 3 && y <= 5) return BLOCK_SAND;
    return BLOCK_GRASS;
}

void chunk_init(struct init_args *args) {
    args->chunk->blocks = args->blocks;
    args->chunk->north = args->north;
    args->chunk->east = args->east;
    args->chunk->south = args->south;
    args->chunk->west = args->west;
    args->chunk->above = args->above;
    args->chunk->below = args->below;

    args->chunk->vbos = vec_init_size(sizeof(*args->chunk->vbos), VoxelEngine_get_atlas_num_textures());

    memcpy(args->chunk->position, args->position, sizeof(ivec3));

    shader = sm_get_shader(SHADER_DEFAULT);

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
            for (int y = 0; y <= fmaxf(3, fminf(height, CHUNK_SIZE_Y)); y++) {
                args->blocks[COORDS_TO_INDEX(x, y, z)] = height_mapper(y);
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

    if (y < 0) {
        if (!chunk->below) return BLOCK_INVALID_ID;
        return chunk_get_block(chunk->below, x, mod(y, CHUNK_SIZE_Y), z);
    }

    if (y >= CHUNK_SIZE_Y) {
        if (!chunk->above) return BLOCK_INVALID_ID;
        return chunk_get_block(chunk->above, x, mod(y, CHUNK_SIZE_Y), z);
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

    unsigned int currentTexture = g_blockData[chunk_get_block(chunk, start[0], start[1], start[2])].sideTextures[orientation];

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
            g_blockData[block].sideTextures[orientation] != currentTexture ||
            (chunk_get_block(chunk, *neighbor[0], *neighbor[1], *neighbor[2]) != BLOCK_INVALID_ID &&
             chunk_get_block(chunk, *neighbor[0], *neighbor[1], *neighbor[2]) != BLOCK_AIR)) {
            break;
        }

        length++;
    }

    for (*widthDimension += 1; *widthDimension < widthLimit; (*widthDimension)++) {
        bool isWholeLineOk = true;
        for (*lengthDimension = startLengthDimension;
             *lengthDimension <= startLengthDimension + length; (*lengthDimension)++) {
            BlockId block = chunk_get_block(chunk, start[0], start[1], start[2]);
            if ((meshedFaces[start[0]][start[1]][start[2]] & orientationMask) == orientationMask ||
                block == BLOCK_INVALID_ID ||
                block == BLOCK_AIR ||
                g_blockData[block].sideTextures[orientation] != currentTexture ||
                (chunk_get_block(chunk, *neighbor[0], *neighbor[1], *neighbor[2]) != BLOCK_INVALID_ID &&
                 chunk_get_block(chunk, *neighbor[0], *neighbor[1], *neighbor[2]) != BLOCK_AIR)) {
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

void chunk_update_mesh(Chunk *chunk, unsigned int targetTexture) {
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
                if ((tempBlock == BLOCK_INVALID_ID || tempBlock == BLOCK_AIR) && (meshedFaces[x][y][z] & FACE_TOP_MASK)
                    !=
                    FACE_TOP_MASK) {
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
                if ((tempBlock == BLOCK_INVALID_ID || tempBlock == BLOCK_AIR) && (
                        meshedFaces[x][y][z] & FACE_BOTTOM_MASK) !=
                    FACE_BOTTOM_MASK) {
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
                if ((tempBlock == BLOCK_INVALID_ID || tempBlock == BLOCK_AIR) && (meshedFaces[x][y][z] & FACE_LEFT_MASK)
                    !=
                    FACE_LEFT_MASK) {
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
                if ((tempBlock == BLOCK_INVALID_ID || tempBlock == BLOCK_AIR) && (
                        meshedFaces[x][y][z] & FACE_RIGHT_MASK) !=
                    FACE_RIGHT_MASK) {
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
                if ((tempBlock == BLOCK_INVALID_ID || tempBlock == BLOCK_AIR) && (
                        meshedFaces[x][y][z] & FACE_FRONT_MASK) !=
                    FACE_FRONT_MASK) {
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
                if ((tempBlock == BLOCK_INVALID_ID || tempBlock == BLOCK_AIR) && (meshedFaces[x][y][z] & FACE_BACK_MASK)
                    !=
                    FACE_BACK_MASK) {
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
    chunk_update_mesh(chunk, 0);
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

    shader_set_mat4(shader, "model", &chunk->model);
    shader_set_int(shader, "TextureUnitId", 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, VoxelEngine_get_atlas_id());
    for (size_t i = 0; i < vec_size(chunk->vbos); i++) {
        if (chunk->vbos[i].vbo == 0) continue;
        glBindBuffer(GL_ARRAY_BUFFER, chunk->vbos[i].vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        shader_set_int(shader, "atlasIndex", i);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

    if (y == 0 && chunk->below != nullptr) {
        chunk_block_updated_at(chunk->below, x, CHUNK_SIZE_Y - 1, z);
    } else if (y == CHUNK_SIZE_Y - 1 && chunk->above != nullptr) {
        chunk_block_updated_at(chunk->above, x, 0, z);
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
