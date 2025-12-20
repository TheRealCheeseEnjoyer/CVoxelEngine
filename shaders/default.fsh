#version 330 core

out vec4 FragColor;
in vec3 TexCoord;

uniform sampler2DArray TextureUnitId;
uniform int atlasIndex;

void main() {
    vec4 texColor = texture(TextureUnitId, TexCoord);
    if (texColor.a < 0.1f)
        discard;
    FragColor = texColor;
    //FragColor = vec4(1, 1, 1, 1);
}