#include "Shader.h"
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int read_file(const char* filePath, const char** output) {
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error opening vertex shader file");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    unsigned int length = ftell(file);
    rewind(file);

    *output = malloc(length * sizeof(char));
    if (!*output) {
        fprintf(stderr, "Error allocating memory. File %s not read.", filePath);
        return 0;
    }
    fread(*output, length, sizeof(char), file);
    fclose(file);
    return length;
}

void check_compile_errors(unsigned int shader, const char* type) {
    GLint success;
    GLchar infoLog[1024];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            fprintf(stderr, "ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- ", type, infoLog);
        }
        return;
    }

    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
        fprintf(stderr, "ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- ", type, infoLog);
    }
}

Shader shader_create(const char* vertexFilePath, const char* fragmentFilePath) {
    const char* vertexCode = nullptr;
    const char* fragmentCode = nullptr;

    unsigned int vertexCodeLength = read_file(vertexFilePath, &vertexCode);
    unsigned int fragmentCodeLength = read_file(fragmentFilePath, &fragmentCode);

    if (vertexCodeLength == 0 || fragmentCodeLength == 0) {
        free(vertexCode);
        free(fragmentCode);
        return 0;
    }

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCode, &vertexCodeLength);
    glCompileShader(vertexShader);
    check_compile_errors(vertexShader, "VERTEX");

    //fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCode, &fragmentCodeLength);
    glCompileShader(fragmentShader);
    check_compile_errors(fragmentShader, "FRAGMENT");

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    check_compile_errors(program, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    free(vertexCode);
    free(fragmentCode);
    return program;
}

void shader_use(Shader shader) {
    glUseProgram(shader);
}

void shader_delete(Shader shader) {
    glDeleteProgram(shader);
}

void shader_set_mat4(Shader shader, const char* name, const mat4* matrix) {
    glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE, (matrix[0][0]));
}

void shader_set_int(Shader shader, const char *name, int value) {
    glUniform1i(glGetUniformLocation(shader, name), value);
}

void shader_set_vec3(Shader shader, const char *name, const vec3 *vector) {
    glUniform3fv(glGetUniformLocation(shader, name), 1, vector[0]);
}
