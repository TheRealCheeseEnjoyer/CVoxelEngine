#include "../include/ShaderManager.h"

#define BASE_PATH "shaders/"

Shader shaders[SHADER_NUM_TYPES] = {0};

const char* files[SHADER_NUM_TYPES][2] = {
    {BASE_PATH "default.vsh", BASE_PATH "default.fsh"},
    {BASE_PATH "skybox.vsh", BASE_PATH "skybox.fsh"},
};

Shader init_shader(ShaderType type) {
    Shader shader = shader_create(files[type][0], files[type][1]);
    shaders[type] = shader;
    return shader;
}

Shader sm_get_shader(ShaderType type) {
    if (shaders[type] == 0)
        return init_shader(type);

    return shaders[type];
}
