#include "managers/TextureManager.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "hashmap.h"
#include "CVector/Vector.h"

static hashmap* textures;
static char** atlasTextures;

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

void tm_init() {
    textures = hashmap_init();
}

void tm_begin_atlas() {
    atlasTextures = vec_init(sizeof(char*));
}

unsigned int tm_add_texture_to_atlas(const char* texture) {
    size_t index;
    for (index = 0; index < vec_size(atlasTextures); index++) {
        if (strcmp(atlasTextures[index], texture) == 0) return index;
    }
    vec_append(&atlasTextures, &texture);
    return index;
}

unsigned int tm_end_atlas() {
    unsigned int atlas;
    glGenTextures(1, &atlas);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);

    stbi_set_flip_vertically_on_load(true);
    const size_t numTextures = vec_size(atlasTextures);
    int width, height, nrChannels;

    unsigned char* textureData[numTextures];
    for (int i = 0; i < numTextures; i++) {
        // suppose sizes are equal for every texture
        textureData[i] = stbi_load(atlasTextures[i], &width, &height, &nrChannels, 4);
    }

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, numTextures, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA, width / 2, height / 2, numTextures, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 2, GL_RGBA, width / 4, height / 4, numTextures, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    for (int i = 0; i < numTextures; i++) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                        textureData[i]);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, i, width / 2, height / 2, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                        textureData[i]);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 2, 0, 0, i, width / 4, height / 4, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                        textureData[i]);
        stbi_image_free(textureData[i]);
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    vec_free(atlasTextures);
    return atlas;
}

int tm_begin_dynamic_atlas(int maxTextureNum, int spriteWidth, int spriteHeight) {
    unsigned int atlas;
    glGenTextures(1, &atlas);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, spriteWidth, spriteHeight, maxTextureNum, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    return atlas;
}

int tm_dynamic_atlas_add_texture(unsigned int atlasId, const char* texture, int index) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlasId);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;

    unsigned char* textureData = stbi_load(texture, &width, &height, &nrChannels, 4);

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    textureData);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    stbi_image_free(textureData);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    return 1;
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
