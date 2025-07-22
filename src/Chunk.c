#include "../include/Chunk.h"

#include <string.h>
#include <cglm/affine.h>
#include <glad/glad.h>

#include "../include/TextureManager.h"
#include "../include/FaceOrientation.h"

#define COORDS_TO_INDEX(x, y, z) x + y * CHUNK_SIZE_X + z * CHUNK_SIZE_X * CHUNK_SIZE_Y
#define X 0
#define Y 1
#define Z 2
int mod(int a, int b) { return (a % b + b) % b; }

void chunk_init(Chunk *chunk, ivec3 position, Chunk *north, Chunk *south, Chunk *east, Chunk *west, Block *blocks) {
    chunk->blocks = blocks;
    chunk->north = north;
    chunk->south = south;
    chunk->east = east;
    chunk->west = west;

    memset(chunk->vbos, 0, sizeof(chunk->vbos));
    memset(chunk->meshes, 0, sizeof(chunk->meshes));

    glm_mat4_identity(chunk->model);
    glm_translate(chunk->model, (vec3){position[X] * CHUNK_SIZE_X, 0, position[Z] * CHUNK_SIZE_Z});

    for (int x = 0; x < CHUNK_SIZE_X; x++) {
        for (int y = 0; y < 2; y++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                blocks[COORDS_TO_INDEX(x, y, z)].type = (y == 0 ? BLOCK_ROCK : BLOCK_GRASS);
            }
        }
    }
}

Block *chunk_get_block(Chunk *chunk, int x, int y, int z) {
    if (y < 0 || y >= CHUNK_SIZE_Y) return nullptr;

    if (x < 0) {
        if (!chunk->east)
            return nullptr;
        return chunk_get_block(chunk->east, mod(x, CHUNK_SIZE_X), y, z);
    }

    if (x >= CHUNK_SIZE_X) {
        if (!chunk->west)
            return nullptr;
        return chunk_get_block(chunk->west, mod(x, CHUNK_SIZE_X), y, z);
    }

    if (z < 0) {
        if (!chunk->south)
            return nullptr;
        return chunk_get_block(chunk->south, x, y, mod(z, CHUNK_SIZE_Z));
    }

    if (z >= CHUNK_SIZE_Z) {
        if (!chunk->north)
            return nullptr;
        return chunk_get_block(chunk->north, x, y, mod(z, CHUNK_SIZE_Z));
    }

    return &chunk->blocks[COORDS_TO_INDEX(x, y, z)];
}

void chunk_get_surface_bounds(Chunk *chunk, ivec3 startPos, Vertex vertices[2], const FaceOrientation orientation,
                              char meshedFaces[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z]) {
    int length = 0, width = 0;
    ivec3 start = {startPos[X], startPos[Y], startPos[Z]};
    int startLengthDimension = 0, startWidthDimension = 0;
    int lengthLimit = 0, widthLimit = 0;
    int *lengthDimension = nullptr;
    int *widthDimension = nullptr;
    int neighborDirection;
    int neighborDirectionIndex = 0;
    int *neighbor[3];
    BlockType currentType = chunk_get_block(chunk, start[X], start[Y], start[Z])->type;

    switch (orientation) {
        case FACE_TOP:
        case FACE_BOTTOM:
            lengthDimension = &start[X];
            widthDimension = &start[Z];
            startLengthDimension = start[X];
            startWidthDimension = start[Z];
            lengthLimit = CHUNK_SIZE_X;
            widthLimit = CHUNK_SIZE_Z;
            neighborDirection = start[Y] + (orientation == FACE_TOP ? 1 : -1);
            neighbor[X] = lengthDimension;
            neighbor[Y] = &neighborDirection;
            neighbor[Z] = widthDimension;
            neighborDirectionIndex = 1;
            vertices[0].position[X] = start[X] - .5f;
            vertices[0].position[Y] = start[Y] + (orientation == FACE_TOP ? .5f : -.5f);
            vertices[0].position[Z] = start[Z] - .5f;
            vertices[1].position[Y] = start[Y] + (orientation == FACE_TOP ? .5f : -.5f);
            vertices[0].texCoords[X] = (orientation == FACE_TOP ? 1 : -1);
            vertices[0].texCoords[Y] = 0;
            vertices[1].texCoords[X] = 0;
            vertices[1].texCoords[Y] = 1;
            break;
        case FACE_LEFT:
        case FACE_RIGHT:
            lengthDimension = &start[Z];
            widthDimension = &start[Y];
            startLengthDimension = start[Z];
            startWidthDimension = start[Y];
            lengthLimit = CHUNK_SIZE_Z;
            widthLimit = CHUNK_SIZE_Y;
            neighborDirection = start[X] + (orientation == FACE_LEFT ? 1 : -1);
            neighbor[X] = &neighborDirection;
            neighbor[Y] = widthDimension;
            neighbor[Z] = lengthDimension;
            neighborDirectionIndex = 0;
            vertices[0].position[X] = start[X] + (orientation == FACE_LEFT ? .5f : -.5f);
            vertices[0].position[Y] = start[Y] - .5f;
            vertices[0].position[Z] = start[Z] - .5f;
            vertices[1].position[X] = start[X] + (orientation == FACE_LEFT ? .5f : -.5f);
            vertices[0].texCoords[X] = (orientation == FACE_LEFT ? 1 : -1);
            vertices[1].texCoords[X] = 0;
            vertices[0].texCoords[Y] = 0;
            vertices[1].texCoords[Y] = 1;
            break;
        case FACE_FRONT:
        case FACE_BACK:
            lengthDimension = &start[X];
            widthDimension = &start[Y];
            startLengthDimension = start[X];
            startWidthDimension = start[Y];
            lengthLimit = CHUNK_SIZE_X;
            widthLimit = CHUNK_SIZE_Y;
            neighborDirection = start[Z] + (orientation == FACE_FRONT ? 1 : -1);
            neighbor[X] = lengthDimension;
            neighbor[Y] = widthDimension;
            neighbor[Z] = &neighborDirection;
            neighborDirectionIndex = 2;
            vertices[0].position[X] = start[X] - .5f;
            vertices[0].position[Y] = start[Y] - .5f;
            vertices[0].position[Z] = start[Z] + (orientation == FACE_FRONT ? .5f : -.5f);
            vertices[1].position[Z] = start[Z] + (orientation == FACE_FRONT ? .5f : -.5f);
            vertices[0].texCoords[X] = 0;
            vertices[0].texCoords[Y] = 0;
            vertices[1].texCoords[X] = 1;
            vertices[1].texCoords[Y] = 1;
            break;
    }

    for (*lengthDimension += 1; *lengthDimension < lengthLimit; (*lengthDimension)++) {
        if ((meshedFaces[start[X]][start[Y]][start[Z]] & orientation) == orientation ||
            chunk_get_block(chunk, start[X], start[Y], start[Z]) == nullptr ||
            chunk_get_block(chunk, start[X], start[Y], start[Z])->type != currentType ||
            chunk_get_block(chunk, *neighbor[X], *neighbor[Y], *neighbor[Z]) != nullptr) {
            break;
        }

        length++;
    }

    for (*widthDimension += 1; *widthDimension < widthLimit; (*widthDimension)++) {
        bool isWholeLineOk = true;
        for (*lengthDimension = startLengthDimension;
             *lengthDimension <= startLengthDimension + length; (*lengthDimension)++) {
            if ((meshedFaces[start[X]][start[Y]][start[Z]] & orientation) == orientation ||
                chunk_get_block(chunk, start[X], start[Y], start[Z]) == nullptr ||
                chunk_get_block(chunk, start[X], start[Y], start[Z])->type != currentType ||
                (chunk_get_block(chunk, *neighbor[X], *neighbor[Y], *neighbor[Z]) != nullptr &&
                 chunk_get_block(chunk, *neighbor[X], *neighbor[Y], *neighbor[Z])->type != BLOCK_AIR)) {
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
            meshedFaces[start[X]][start[Y]][start[Z]] ^= orientation;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (i == neighborDirectionIndex) continue;
        vertices[1].position[i] = *neighbor[i] -.5f;
    }

    vertices[0].texCoords[X] *= length + 1;
    vertices[0].texCoords[Y] *= width + 1;
    vertices[1].texCoords[X] *= length + 1;
    vertices[1].texCoords[Y] *= width + 1;
}

void chunk_create_mesh(Chunk *chunk) {
    char meshedFaces[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z] = {0};

    for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
        for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
            for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                Block *block = chunk_get_block(chunk, x, y, z);
                if (block == nullptr || block->type == BLOCK_AIR) continue;

                if (chunk->meshes[block->type] == nullptr) {
                    chunk->meshes[block->type] = vec_init(sizeof(Vertex));
                }

                Vector mesh = chunk->meshes[block->type];
                ivec3 blockPosition = {x, y, z};
                Block *tempBlock = chunk_get_block(chunk, x, y + 1, z);
                Vertex vertices[2];
                if ((tempBlock == nullptr || tempBlock->type == BLOCK_AIR) && (meshedFaces[x][y][z] & FACE_TOP) !=
                    FACE_TOP) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_TOP, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[0].position[X], vertices[0].position[Y], vertices[1].position[Z]},
                        .texCoords = {vertices[0].texCoords[X], vertices[1].texCoords[Y]}
                    };
                    Vertex v2 = {
                        .position = {vertices[1].position[X], vertices[1].position[Y], vertices[0].position[Z]},
                        .texCoords = {vertices[1].texCoords[X], vertices[0].texCoords[Y]}
                    };
                    vec_append(mesh, &vertices[0]);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x, y - 1, z);
                if ((tempBlock == nullptr || tempBlock->type == BLOCK_AIR) && (meshedFaces[x][y][z] & FACE_BOTTOM) !=
                    FACE_BOTTOM) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_BOTTOM, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[1].position[X], vertices[0].position[Y], vertices[0].position[Z]},
                        .texCoords = {vertices[1].texCoords[X], vertices[0].texCoords[Y]}
                    };
                    Vertex v2 = {
                        .position = {vertices[0].position[X], vertices[0].position[Y], vertices[1].position[Z]},
                        .texCoords = {vertices[0].texCoords[X], vertices[1].texCoords[Y]}
                    };
                    vec_append(mesh, &vertices[0]);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x + 1, y, z);
                if ((tempBlock == nullptr || tempBlock->type == BLOCK_AIR) && (meshedFaces[x][y][z] & FACE_LEFT) !=
                    FACE_LEFT) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_LEFT, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[0].position[X], vertices[1].position[Y], vertices[0].position[Z]},
                        .texCoords = {vertices[0].texCoords[X], vertices[1].texCoords[Y]}
                    };
                    Vertex v2 = {
                        .position = {vertices[0].position[X], vertices[0].position[Y], vertices[1].position[Z]},
                        .texCoords = {vertices[1].texCoords[X], vertices[0].texCoords[Y]}
                    };

                    vec_append(mesh, &vertices[0]);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x - 1, y, z);
                if ((tempBlock == nullptr || tempBlock->type == BLOCK_AIR) && (meshedFaces[x][y][z] & FACE_RIGHT) !=
                    FACE_RIGHT) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_RIGHT, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[0].position[X], vertices[0].position[Y], vertices[1].position[Z]},
                        .texCoords = {vertices[1].texCoords[X], vertices[0].texCoords[Y]}
                    };
                    Vertex v2 = {
                        .position = {vertices[0].position[X], vertices[1].position[Y], vertices[0].position[Z]},
                        .texCoords = {vertices[0].texCoords[X], vertices[1].texCoords[Y]}
                    };

                    vec_append(mesh, &vertices[0]);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x, y, z + 1);
                if ((tempBlock == nullptr || tempBlock->type == BLOCK_AIR) && (meshedFaces[x][y][z] & FACE_FRONT) !=
                    FACE_FRONT) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_FRONT, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[1].position[X], vertices[0].position[Y], vertices[0].position[Z]},
                        .texCoords = {vertices[1].texCoords[X], vertices[0].texCoords[Y]}
                    };
                    Vertex v2 = {
                        .position = {vertices[0].position[X], vertices[1].position[Y], vertices[0].position[Z]},
                        .texCoords = {vertices[0].texCoords[X], vertices[1].texCoords[Y]}
                    };

                    vec_append(mesh, &vertices[0]);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &vertices[1]);
                }

                tempBlock = chunk_get_block(chunk, x, y, z - 1);
                if ((tempBlock == nullptr || tempBlock->type == BLOCK_AIR) && (meshedFaces[x][y][z] & FACE_BACK) !=
                    FACE_BACK) {
                    chunk_get_surface_bounds(chunk, blockPosition, vertices, FACE_BACK, meshedFaces);
                    Vertex v1 = {
                        .position = {vertices[0].position[X], vertices[1].position[Y], vertices[0].position[Z]},
                        .texCoords = {vertices[0].texCoords[X], vertices[1].texCoords[Y]}
                    };
                    Vertex v2 = {
                        .position = {vertices[1].position[X], vertices[0].position[Y], vertices[0].position[Z]},
                        .texCoords = {vertices[1].texCoords[X], vertices[0].texCoords[Y]}
                    };

                    vec_append(mesh, &vertices[0]);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v2);
                    vec_append(mesh, &v1);
                    vec_append(mesh, &vertices[1]);
                }
            }
        }
    }
}

void chunk_load_mesh(Chunk *chunk) {
    if (chunk->VAO == 0)
        glGenVertexArrays(1, &chunk->VAO);

    glBindVertexArray(chunk->VAO);

    for (int i = 0; i < BLOCK_NUM_BLOCK_TYPES; i++) {
        if (chunk->meshes[i] == nullptr || chunk->vbos[i] != 0) continue;
        glGenBuffers(1, &chunk->vbos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (vec_size(chunk->meshes[i]) * sizeof(Vertex)),
                     vec_get(chunk->meshes[i], 0), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glBindVertexArray(0);
}

void chunk_draw(Chunk *chunk, Shader shader, mat4 projection, mat4 view) {
    glBindVertexArray(chunk->VAO);
    glActiveTexture(GL_TEXTURE0);
    mat4 final;
    glm_mat4_mul(projection, view, final);
    glm_mat4_mul(final, chunk->model, final);
    shader_set_mat4(shader, "finalMatrix", &final);
    shader_set_int(shader, "TextureUnitId", 0);

    for (int i = 1; i < BLOCK_NUM_BLOCK_TYPES; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, chunk->vbos[i]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindTexture(GL_TEXTURE_2D, tm_get_texture_id(i));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, (int) vec_size(chunk->meshes[i]));
    }
}
