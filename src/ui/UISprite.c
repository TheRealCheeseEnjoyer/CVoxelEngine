#include "ui/UISprite.h"
#include "managers/ShaderManager.h"
#include "managers/TextureManager.h"
#include <cglm/affine.h>
#include <glad/glad.h>

#define INITIAL_SIZE 32
#define RESIZE_FACTOR 2

typedef struct {
    mat4 transform;
    unsigned int texture;
} uisprite_t;

static uisprite_t* sprites = nullptr;
static int len, capacity;

void init() {
    sprites = (uisprite_t*)malloc(sizeof(uisprite_t) * INITIAL_SIZE);
    capacity = INITIAL_SIZE;
    len = 0;
}

void resize() {
    int newCapacity = capacity * RESIZE_FACTOR;
    void* newMem = (uisprite_t*)realloc(sprites, newCapacity * sizeof(uisprite_t));
    if (!newMem) {
        exit(-1);
    }

    capacity = newCapacity;
    sprites = newMem;
}

UISprite UISprite_init(const char *texture, vec2 position, vec2 size) {
    if (!sprites)
        init();
    if (len + 1 >= capacity)
        resize();

    glm_mat4_identity(sprites[len].transform);
    glm_translate(sprites[len].transform, (vec3) {position[0] - size[0] / 2, position[1] - size[1] / 2, 0});
    glm_scale(sprites[len].transform, (vec3) {size[0], size[1], 1});
    sprites[len].texture = tm_get_texture_id(texture);

    return len++;
}

void UISprite_set_position(UISprite spriteIndex, vec2 position) {
    vec3 scale;
    mat4 rot;
    glm_decompose_rs(sprites[spriteIndex].transform, rot, scale);
    glm_mat4_identity(sprites[spriteIndex].transform);
    glm_translate(sprites[spriteIndex].transform, (vec3) {position[0] - scale[0] / 2, position[1] - scale[1] / 2, 0});
    glm_scale(sprites[spriteIndex].transform, scale);
}

void UISprite_set_texture(UISprite spriteIndex, const char *texture) {
    sprites[spriteIndex].texture = tm_get_texture_id(texture);
}

void UISprite_get_size(UISprite spriteIndex, vec2 size) {
    vec3 scale;
    glm_decompose_scalev(sprites[spriteIndex].transform, scale);
    size[0] = scale[0];
    size[1] = scale[1];
}

void UISprite_get_position(UISprite spriteIndex, vec2 position) {
    vec2 size;
    UISprite_get_size(spriteIndex, size);

    position[0] = sprites[spriteIndex].transform[3][0] + size[0] / 2;
    position[1] = sprites[spriteIndex].transform[3][1] + size[1] / 2;
}

void UISprite_draw(UISprite spriteIndex) {
    shader_set_mat4(sm_get_shader(SHADER_UI), "model", &sprites[spriteIndex].transform);
    glBindTexture(GL_TEXTURE_2D, sprites[spriteIndex].texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
