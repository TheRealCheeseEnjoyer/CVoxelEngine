#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D TextureUnitId;
uniform vec3 textColor;

void main() {
    FragColor = vec4(textColor, 1) * vec4(1, 1, 1, texture(TextureUnitId, TexCoord).r);
}