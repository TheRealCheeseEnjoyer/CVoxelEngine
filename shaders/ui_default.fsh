#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec2 TexCoord;
    flat int index;
} fs_in;

uniform sampler2DArray TextureUnitId;
uniform bool enabled[128];

void main() {
    if (!enabled[fs_in.index]) discard;
    FragColor = texture(TextureUnitId, vec3(fs_in.TexCoord, fs_in.index));
}