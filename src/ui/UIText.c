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

void UIText_init(UIText *ui_text, const char *text, vec2 position, bool enabled) {
    ui_text->enabled = enabled;
    ui_text->len = strlen(text);
    ui_text->chars = calloc(ui_text->len, sizeof(UIChar));
    for (int i = 0; i < strlen(text); i++) {
        char c = text[i];
        CharGlyph glyph;
        glyph_manager_get_glyph(c, &glyph);
        float xPos = position[0] + glyph.bearing[0];
        float yPos = position[1] + ( - glyph.bearing[1]);

        float w = glyph.size[0];
        float h = glyph.size[1];

        float vertices[6][4] = {
            {xPos, yPos + h, 0.0f, 0.0f},
            {xPos, yPos, 0.0f, 1.0f},
            {xPos + w, yPos, 1.0f, 1.0f},
            {xPos, yPos + h, 0.0f, 0.0f},
            {xPos + w, yPos, 1.0f, 1.0f},
            {xPos + w, yPos + h, 1.0f, 0.0f}
        };

        ui_text->chars[i].texture = glyph.texture;

        glm_mat4_identity(ui_text->chars[i].transform);
        glm_translate(ui_text->chars[i].transform, (vec3) {xPos, yPos, 0});
        glm_scale(ui_text->chars[i].transform, (vec3) {w, h, 1});
        position[0] += glyph.advance >> 6;
    }
}

void UIText_draw(UIText* ui_text) {
    if (!ui_text->enabled) return;
    shader_set_int(sm_get_shader(SHADER_UI), "text", true);
    for (int i = 0; i < ui_text->len; i++) {
        shader_set_mat4(sm_get_shader(SHADER_UI), "model", &ui_text->chars[i].transform);
        glBindTexture(GL_TEXTURE_2D, ui_text->chars[i].texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    shader_set_int(sm_get_shader(SHADER_UI), "text", false);
}
