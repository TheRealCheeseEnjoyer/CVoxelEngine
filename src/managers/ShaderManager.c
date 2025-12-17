#include "managers/ShaderManager.h"

#define BASE_PATH "shaders/"

Shader shaders[SHADER_NUM_TYPES] = {0};

const char* files[SHADER_NUM_TYPES][2] = {
    {BASE_PATH "default.vsh", BASE_PATH "default.fsh"},
    {BASE_PATH "skybox.vsh", BASE_PATH "skybox.fsh"},
    {BASE_PATH "ui_default.vsh", BASE_PATH "ui_default.fsh"},
    {BASE_PATH "ui_text.vsh", BASE_PATH "ui_text.fsh"},
    {BASE_PATH "ui_default.vsh", BASE_PATH "ui_3d.fsh"}
};

Shader init_shader(ShaderType type) {
    Shader shader = shader_create(files[type][0], files[type][1]);
    shaders[type] = shader;
    return shader;
}

void sm_init() {
    for (int i = 0; i < SHADER_NUM_TYPES; i++) {
        shaders[i] = init_shader(i);
    }
}

Shader sm_get_shader(ShaderType type) {
    if (type >= SHADER_NUM_TYPES)
        return 0;

    return shaders[type];
}
