#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2DArray TextureUnitId;
uniform int atlasIndex;

void main() {
    FragColor = texture(TextureUnitId, vec3(TexCoord, atlasIndex));
    //FragColor = vec4(1, 1, 1, 1);
}