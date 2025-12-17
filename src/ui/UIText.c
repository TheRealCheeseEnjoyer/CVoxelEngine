#include "ui/UIText.h"

#include <string.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <glad/glad.h>

#include "CommonVertices.h"
#include "Shader.h"
#include "CVector/Vector.h"
#include "managers/GlyphManager.h"
#include "managers/ShaderManager.h"
#include "ui/CharGlyph.h"
#include "ui/UIManager.h"

static constexpr unsigned int MaxTextLen = 64;

typedef struct {
    vec2 position;
    vec3 color;
    mat4 transforms[MaxTextLen];
    int letterMap[MaxTextLen];
    size_t textLen;
    bool enabled;
} uitext_t;

static uitext_t* texts = nullptr;
static int modelLocation;
static int textureLocation;
static unsigned int vao, vbo;

UIText UIText_init(const char *text, vec2 position, bool enabled) {
    if (!texts) {
        texts = vec_init(sizeof(uitext_t));
        glyph_manager_init_glyphs("assets/fonts/COMIC.ttf");
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices_strip), quadVertices_strip, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        shader_use(sm_get_shader(SHADER_UI_TEXT));
        mat4 orthoMatrix;
        UIManager_get_ortho_matrix(orthoMatrix);
        shader_set_mat4(sm_get_shader(SHADER_UI_TEXT), "ortho", &orthoMatrix);
        shader_set_int(sm_get_shader(SHADER_UI_TEXT), "TextureUnitId", UIManager_get_active_texture_id());
        modelLocation = glGetUniformLocation(sm_get_shader(SHADER_UI_TEXT), "model");
        textureLocation = glGetUniformLocation(sm_get_shader(SHADER_UI_TEXT), "textureIndex");
    }

    vec_append_empty(&texts);
    UIText id = vec_size(texts) - 1;
    texts[id].position[0] = position[0];
    texts[id].position[1] = position[1];
    texts[id].color[0] = 0;
    texts[id].color[1] = 0;
    texts[id].color[2] = 0;
    UIText_set_text(id, text);
    texts[id].enabled = enabled;
    return id;
}

void UIText_set_enabled(UIText text, bool enabled) {
    texts[text].enabled = enabled;
}

/*void update_position(UIText textId) {
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
}*/

void UIText_set_position(UIText ui_text, vec2 position) {
    texts[ui_text].position[0] = position[0];
    texts[ui_text].position[1] = position[1];
    //update_position(ui_text);
}

void UIText_set_color(UIText ui_text, vec3 color) {
    texts[ui_text].color[0] = color[0];
    texts[ui_text].color[1] = color[1];
    texts[ui_text].color[2] = color[2];
}

void UIText_set_text(UIText textIndex, const char *text) {
    float scale = 48.f / 256.f;
    int workingIndex = 0;
    int posX = texts[textIndex].position[0];
    for (int i = 0; i < fmin(strlen(text), MaxTextLen); i++) {
        char c = text[i];
        CharGlyph glyph;
        glyph_manager_get_glyph(c, &glyph);

        float xPos = posX + glyph.bearing[0] * scale;
        float yPos = texts[textIndex].position[1] + (256 - glyph.bearing[1]) * scale;

        glm_mat4_identity(texts[textIndex].transforms[workingIndex]);
        glm_translate(texts[textIndex].transforms[workingIndex], (vec3) {xPos, yPos, 0});
        glm_scale(texts[textIndex].transforms[workingIndex], (vec3) {256 * scale, 256 * scale, 1});
        texts[textIndex].letterMap[workingIndex] = glyph.character;
        posX += (glyph.advance >> 6) * scale;
        workingIndex++;
    }
    texts[textIndex].textLen = workingIndex;
}

void UIText_draw() {
    shader_use(sm_get_shader(SHADER_UI_TEXT));
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0 + UIManager_get_active_texture_id());
    glBindTexture(GL_TEXTURE_2D_ARRAY, glyph_manager_get_atlas_id());
    for (int i = 0; i < vec_size(texts); i++) {
        if (!texts[i].enabled) continue;
        shader_set_vec3(sm_get_shader(SHADER_UI_TEXT), "textColor", &texts[i].color);
        glUniformMatrix4fv(modelLocation, texts[i].textLen, GL_FALSE, texts[i].transforms[0][0]);
        glUniform1iv(glGetUniformLocation(sm_get_shader(SHADER_UI_TEXT), "letterMap"), texts[i].textLen, texts[i].letterMap);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, texts[i].textLen);
    }
    glBindVertexArray(0);
}
