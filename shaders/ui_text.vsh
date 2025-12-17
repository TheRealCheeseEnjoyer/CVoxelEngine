#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;

out VS_OUT {
    vec2 TexCoord;
    flat int index;
} vs_out;
uniform mat4 model[64];
uniform mat4 ortho;

void main() {
    gl_Position = ortho * model[gl_InstanceID] * vec4(aPos, 1);
    vs_out.index = gl_InstanceID;
    vs_out.TexCoord = texCoord;
}