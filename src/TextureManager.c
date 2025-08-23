#include "../include/TextureManager.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

unsigned int textures[BLOCK_NUM_BLOCK_TYPES] = { 0 };

void generateTexture(BlockType type) {
    unsigned int texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = nullptr;
    switch (type) {
        case BLOCK_GRASS:
            data = stbi_load("assets/grass.png", &width, &height, &nrChannels, 0);
            break;
        case BLOCK_ROCK:
            data = stbi_load("assets/rock.png", &width, &height, &nrChannels, 0);
            break;
        case BLOCK_WATER:
            data = stbi_load("assets/water.png", &width, &height, &nrChannels, 0);
            break;
        case BLOCK_SAND:
            data = stbi_load("assets/sand.png", &width, &height, &nrChannels, 0);
            break;
        case BLOCK_WOOD:
            data = stbi_load("assets/wood.png", &width, &height, &nrChannels, 0);
        default:
            break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    textures[type] = texture;
}

unsigned int tm_get_texture_id(BlockType type) {
    if (textures[type] == 0) {
        generateTexture(type);
    }

    return textures[type];
}
