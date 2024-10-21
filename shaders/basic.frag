#version 460 core

in VS_OUT {
    flat int material_index;
    vec2 uv;
} fs_in;

uniform sampler2DArray texture_array;

out vec4 frag_color;

void main() {
    frag_color = texture(texture_array, vec3(fs_in.uv, float(fs_in.material_index * 3)));
}