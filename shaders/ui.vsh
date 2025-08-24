#version 330 core

layout (location = 0) in vec2 aPos;

out vec2 TexCoord;
uniform mat4 model;
uniform mat4 ortho;

void main() {
    gl_Position = ortho * model * vec4(aPos.x, aPos.y, 1, 1);
    TexCoord = aPos;
}