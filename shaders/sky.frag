//FRAGMENT_SHADER
#version 460 core
in VS_OUT {
    smooth vec3 model_space_position;
} fs_in;

uniform samplerCube cubemap;

out vec4 frag_color;

void main() {
    frag_color = texture(cubemap, fs_in.model_space_position);
}