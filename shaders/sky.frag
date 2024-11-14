//FRAGMENT_SHADER
#version 460 core
in VS_OUT {
    vec3 model_space_position;
} fs_in;

layout(binding = 1) uniform samplerCube cube_map;

out vec4 frag_color;

void main() {
    frag_color = texture(cube_map, fs_in.model_space_position);
}