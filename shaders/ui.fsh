#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D TextureUnitId;

void main() {
    //FragColor = vec4(1, 1, 1, 1);
    FragColor = texture(TextureUnitId, TexCoord);
}