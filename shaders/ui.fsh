#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D TextureUnitId;
uniform int text;

void main() {
    if (text == 0)
        FragColor = texture(TextureUnitId, TexCoord);
    else
        FragColor = vec4(1, 1, 1, texture(TextureUnitId, TexCoord).r);
}