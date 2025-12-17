#include "Skybox.h"

#include <string.h>
#include <cglm/affine.h>
#include <glad/glad.h>

#include "Player.h"
#include "Shader.h"
#include "../include/managers/ShaderManager.h"
#include "../libs/stb_image.h"

float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f
};

#define BASE_DIR "assets/skybox/"
#define FRONT_SUFFIX "_ft.jpg"
#define BACK_SUFFIX "_bk.jpg"
#define DOWN_SUFFIX "_dn.jpg"
#define UP_SUFFIX "_up.jpg"
#define RIGHT_SUFFIX "_rt.jpg"
#define LEFT_SUFFIX "_lf.jpg"

const char *suffixes[] = {
    FRONT_SUFFIX,
    BACK_SUFFIX,
    DOWN_SUFFIX,
    UP_SUFFIX,
    RIGHT_SUFFIX,
    LEFT_SUFFIX
};

static unsigned int vax, vbo;
static unsigned int textureId;
static Shader shader;

void skybox_init(const char *name) {
    char *faces[6];
    for (int i = 0; i < sizeof(faces) / sizeof(faces[0]); i++) {
        faces[i] = calloc(strlen(BASE_DIR) + strlen(name) + strlen(suffixes[i]) + 1, sizeof(char));
        strcat(faces[i], BASE_DIR);
        strcat(faces[i], name);
        strcat(faces[i], suffixes[i]);
    }

    glGenVertexArrays(1, &vax);
    glBindVertexArray(vax);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++) {
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        stbi_set_flip_vertically_on_load(true);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        } else {
            fprintf(stderr, "Cubemap texture failed to load at path: %s\n", faces[i]);
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    shader = sm_get_shader(SHADER_SKYBOX);
    glBindVertexArray(0);

    for (int i = 0; i < sizeof(faces) / sizeof(faces[0]); i++)
        free(faces[i]);
}

void skybox_draw() {
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(vax);
    vec3 eyePos;
    player_get_eye_position(eyePos);
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, eyePos);
    shader_use(shader);
    mat4 finalMatrix;
    mat4 projection;
    mat4 view;
    player_get_projection(projection);
    player_get_view(view);
    glm_mat4_mul(projection, view, finalMatrix);
    glm_mat4_mul(finalMatrix, model, finalMatrix);

    shader_set_mat4(shader, "final", &finalMatrix);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
    glBindVertexArray(0);
}

void skybox_destroy() {
    shader_delete(shader);
    glDeleteVertexArrays(1, &vax);
    glDeleteBuffers(1, &vbo);
}
