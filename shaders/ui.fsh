#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D TextureUnitId;

void main() {
    FragColor = texture(TextureUnitId, TexCoord);
}