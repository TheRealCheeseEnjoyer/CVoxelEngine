#include "../include/managers/TextureManager.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "hashmap.h"

static hashmap* textures;
static hashmap* atlasTextures;
static unsigned int atlasTextureIndex;
static unsigned int atlas;

void generateTexture(const char* name) {
    unsigned int texture;
    glGenTextures(1, &texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    stbi_set_flip_vertically_on_load(false);
    int width, height, nrChannels;

    unsigned char* data = stbi_load(name, &width, &height, &nrChannels, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    hashmap_set(textures, name, texture);
}

void generateAtlas() {
    glGenTextures(1, &atlas);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);

    stbi_set_flip_vertically_on_load(false);
    int width, height, nrChannels;

    int numTextures = 4;

    unsigned char* data = stbi_load("assets/atlas.png", &width, &height, &nrChannels, 4);
    unsigned char* textureData[numTextures];
    for (int i = 0; i < numTextures; i++) {
        textureData[i] = calloc(width / 2 * nrChannels * height / 2, sizeof(unsigned char));
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width * nrChannels; j++) {
            int y = i / (height / 2);
            int x = j / (width * nrChannels / 2);

            textureData[y * 2 + x][i % (height / 2) * width * nrChannels / 2 + j % (width * nrChannels / 2)] = data[i *
                (width * nrChannels) + j];
        }
    }

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width / 2, height / 2, numTextures, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA, width / 4, height / 4, numTextures, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 2, GL_RGBA, width / 8, height / 8, numTextures, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    for (int i = 0; i < numTextures; i++) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width / 2, height / 2, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                        textureData[i]);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, i, width / 4, height / 4, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                        textureData[i]);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 2, 0, 0, i, width / 8, height / 8, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                        textureData[i]);
        free(textureData[i]);
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    stbi_image_free(data);
}

void tm_init() {
    textures = hashmap_init();
    generateAtlas();
}

void tm_begin_atlas() {
    atlasTextures = hashmap_init();
    atlasTextureIndex = 0;
}

unsigned int tm_add_texture_to_atlas(const char* texture) {
    auto result = hashmap_get(atlasTextures, texture);
    if (result == nullptr) {
        hashmap_set(atlasTextures, texture, atlasTextureIndex);
        return atlasTextureIndex++;
    }

    return result->value;
}

unsigned int tm_end_atlas() {
    // TODO:
    // merge all textures in big atlas
    // delete hashmap
    // generate atlas
    // return atlas
}

void tm_destroy() {
    hashmap_free(textures);
}

unsigned int tm_get_texture_id(const char* name) {
    if (name == nullptr)
        return 0;
    if (hashmap_get(textures, name) == NULL) {
        generateTexture(name);
    }

    return hashmap_get(textures, name)->value;
}

unsigned int tm_get_atlas() {
    return atlas;
}
