#version 330 core

out vec4 FragColor;
in VS_OUT {
    vec2 TexCoord;
    flat int index;
} fs_in;

uniform sampler2DArray TextureUnitId;
uniform int letterMap[400];
uniform vec3 textColor;

void main() {
    FragColor = vec4(textColor, 1) * vec4(1, 1, 1, texture(TextureUnitId, vec3(fs_in.TexCoord, letterMap[fs_in.index])).r);
}