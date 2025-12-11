#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2DArray atlas;
uniform int atlasIndex;

void main() {
    FragColor = texture(atlas, vec3(TexCoord, atlasIndex));
}