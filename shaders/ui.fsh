#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D TextureUnitId;
uniform bool text;
uniform vec3 textColor;

void main() {
    if (!text)
        FragColor = texture(TextureUnitId, TexCoord);
    else
        FragColor = vec4(textColor, 1) * vec4(1, 1, 1, texture(TextureUnitId, TexCoord).r);
}