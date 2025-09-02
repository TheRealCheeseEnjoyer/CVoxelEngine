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
    ui_text->position[0] = position[0];
    ui_text->position[1] = position[1];
    UIText_set_text(ui_text, text);
}


void UIText_set_text(UIText *ui_text, const char *text) {
    free(ui_text->chars);
    ui_text->len = strlen(text);
    ui_text->chars = calloc(ui_text->len, sizeof(UIChar));
    int posX = ui_text->position[0];
    for (int i = 0; i < strlen(text); i++) {
        char c = text[i];
        CharGlyph glyph;
        glyph_manager_get_glyph(c, &glyph);
        float xPos = posX + glyph.bearing[0];
        float yPos = ui_text->position[1] + ( - glyph.bearing[1]);

        ui_text->chars[i].texture = glyph.texture;

        glm_mat4_identity(ui_text->chars[i].transform);
        glm_translate(ui_text->chars[i].transform, (vec3) {xPos, yPos, 0});
        glm_scale(ui_text->chars[i].transform, (vec3) {glyph.size[0], glyph.size[1], 1});
        posX += glyph.advance >> 6;
    }
}

void UIText_draw(UIText* ui_text) {
    if (!ui_text->enabled) return;
    shader_set_int(sm_get_shader(SHADER_UI), "text", true);
    vec3 color = {0, 0, 0};
    shader_set_vec3(sm_get_shader(SHADER_UI), "textColor", &color);
    for (int i = 0; i < ui_text->len; i++) {
        shader_set_mat4(sm_get_shader(SHADER_UI), "model", &ui_text->chars[i].transform);
        glBindTexture(GL_TEXTURE_2D, ui_text->chars[i].texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    shader_set_int(sm_get_shader(SHADER_UI), "text", false);
}
