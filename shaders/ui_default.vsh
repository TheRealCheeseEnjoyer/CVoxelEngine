#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;
uniform mat4 model;
uniform mat4 ortho;

void main() {
    gl_Position = ortho * model * vec4(aPos, 1);
    TexCoord = texCoord;
}