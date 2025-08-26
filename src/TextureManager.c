#include "../include/TextureManager.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include "../include/hashmap.h"

hashmap* textures;

void generateTexture(const char* name) {
    unsigned int texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(name, &width, &height, &nrChannels, 0);

    if (nrChannels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    else if (nrChannels == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    hashmap_set(textures, name, texture);
}

void tm_init() {
    textures = hashmap_init();
}

void tm_destroy() {
    hashmap_free(textures);
}

unsigned int tm_get_texture_id(const char* name) {
    if (hashmap_get(textures, name) == NULL) {
        generateTexture(name);
    }

    return hashmap_get(textures, name)->value;
}
