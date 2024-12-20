//FRAGMENT_SHADER
#version 460 core
in VS_OUT {
    vec4 model_space_position;
} fs_in;

layout (binding = SAMPLER_CUBE_SKY) uniform samplerCube sky_cube_map;

out vec4 frag_color;

void main() {
    frag_color = texture(sky_cube_map, fs_in.model_space_position.xyz);
}