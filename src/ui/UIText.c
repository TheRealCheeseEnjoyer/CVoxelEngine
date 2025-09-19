#include "ui/UIText.h"

#include <stdlib.h>
#include <string.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <glad/glad.h>

#include "Shader.h"
#include "Vector.h"
#include "managers/GlyphManager.h"
#include "managers/ShaderManager.h"
#include "ui/CharGlyph.h"

#define INITIAL_SIZE 32
#define RESIZE_FACTOR 2

typedef struct {
    mat4 transform;
    unsigned int texture;
    char c;
} UIChar;

typedef struct {
    vec2 position;
    UIChar* chars;
} uitext_t;

static uitext_t* texts = nullptr;

UIText UIText_init(const char *text, vec2 position, bool enabled) {
    if (!texts)
        texts = vec_init(sizeof(uitext_t));

    vec_append(&texts, nullptr);
    UIText id = vec_size(texts) - 1;
    texts[id].position[0] = position[0];
    texts[id].position[1] = position[1];
    texts[id].chars = vec_init(sizeof(UIChar));
    UIText_set_text(id, text);
    return id;
}

void update_position(UIText textId) {
    int posX = texts[textId].position[0];
    for (int i = 0; i < vec_size(texts[textId].chars); i++) {
        CharGlyph glyph;
        glyph_manager_get_glyph(texts[textId].chars[i].c, &glyph);
        float xPos = posX + glyph.bearing[0];
        float yPos = texts[textId].position[1] - glyph.bearing[1];

        glm_mat4_identity(texts[textId].chars[i].transform);
        glm_translate(texts[textId].chars[i].transform, (vec3) {xPos, yPos, 0});
        glm_scale(texts[textId].chars[i].transform, (vec3) {glyph.size[0], glyph.size[1], 1});

        posX += glyph.advance >> 6;
    }
}

void UIText_set_position(UIText ui_text, vec2 position) {
    texts[ui_text].position[0] = position[0];
    texts[ui_text].position[1] = position[1];
    update_position(ui_text);
}

void UIText_set_text(UIText textIndex, const char *text) {
    vec_clear(texts[textIndex].chars);

    int posX = texts[textIndex].position[0];
    for (int i = 0; i < strlen(text); i++) {
        char c = text[i];
        CharGlyph glyph;
        glyph_manager_get_glyph(c, &glyph);
        float xPos = posX + glyph.bearing[0];
        float yPos = texts[textIndex].position[1] - glyph.bearing[1];
        UIChar ch;
        ch.c = c;
        ch.texture = glyph.texture;

        glm_mat4_identity(ch.transform);
        glm_translate(ch.transform, (vec3) {xPos, yPos, 0});
        glm_scale(ch.transform, (vec3) {glyph.size[0], glyph.size[1], 1});
        vec_append(&texts[textIndex].chars, &ch);
        posX += glyph.advance >> 6;
    }
}

void UIText_draw(UIText textIndex) {
    shader_set_int(sm_get_shader(SHADER_UI), "text", true);
    vec3 color = {0, 0, 0};
    shader_set_vec3(sm_get_shader(SHADER_UI), "textColor", &color);
    for (int i = 0; i < vec_size(texts[textIndex].chars); i++) {
        shader_set_mat4(sm_get_shader(SHADER_UI), "model", &texts[textIndex].chars[i].transform);
        glBindTexture(GL_TEXTURE_2D, texts[textIndex].chars[i].texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    shader_set_int(sm_get_shader(SHADER_UI), "text", false);
}
