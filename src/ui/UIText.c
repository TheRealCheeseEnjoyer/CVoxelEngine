#include "ui/UIText.h"

#include <stdlib.h>
#include <string.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <glad/glad.h>

#include "Shader.h"
#include "managers/GlyphManager.h"
#include "managers/ShaderManager.h"
#include "ui/CharGlyph.h"

#define INITIAL_SIZE 32
#define RESIZE_FACTOR 2

typedef struct {
    mat4 transform;
    unsigned int texture;
} UIChar;

typedef struct {
    UIChar* chars;
    vec2 position;
    size_t len;
} uitext_t;

static uitext_t* texts = nullptr;
static int capacity, len;

static void init() {
    texts = (uitext_t*)malloc(sizeof(uitext_t) * INITIAL_SIZE);
    capacity = INITIAL_SIZE;
    len = 0;
}

static void resize() {
    int newCapacity = capacity * RESIZE_FACTOR;
    void* newMem = (uitext_t*)realloc(texts, newCapacity * sizeof(uitext_t));
    if (!newMem) {
        exit(-1);
    }

    capacity = newCapacity;
    texts = newMem;
}

UIText UIText_init(const char *text, vec2 position, bool enabled) {
    if (!texts)
        init();
    if (len >= capacity)
        resize();

    texts[len].position[0] = position[0];
    texts[len].position[1] = position[1];
    texts[len].chars = nullptr;
    UIText_set_text(len, text);
    return len++;
}


void UIText_set_text(UIText textIndex, const char *text) {
    free(texts[textIndex].chars);
    texts[textIndex].len = strlen(text);
    texts[textIndex].chars = calloc(texts[textIndex].len, sizeof(UIChar));
    int posX = texts[textIndex].position[0];
    for (int i = 0; i < strlen(text); i++) {
        char c = text[i];
        CharGlyph glyph;
        glyph_manager_get_glyph(c, &glyph);
        float xPos = posX + glyph.bearing[0];
        float yPos = texts[textIndex].position[1] - glyph.bearing[1];

        texts[textIndex].chars[i].texture = glyph.texture;

        glm_mat4_identity(texts[textIndex].chars[i].transform);
        glm_translate(texts[textIndex].chars[i].transform, (vec3) {xPos, yPos, 0});
        glm_scale(texts[textIndex].chars[i].transform, (vec3) {glyph.size[0], glyph.size[1], 1});
        posX += glyph.advance >> 6;
    }
}

void UIText_draw(UIText textIndex) {
    shader_set_int(sm_get_shader(SHADER_UI), "text", true);
    vec3 color = {0, 0, 0};
    shader_set_vec3(sm_get_shader(SHADER_UI), "textColor", &color);
    for (int i = 0; i < texts[textIndex].len; i++) {
        shader_set_mat4(sm_get_shader(SHADER_UI), "model", &texts[textIndex].chars[i].transform);
        glBindTexture(GL_TEXTURE_2D, texts[textIndex].chars[i].texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    shader_set_int(sm_get_shader(SHADER_UI), "text", false);
}
