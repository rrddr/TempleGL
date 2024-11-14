//FRAGMENT_SHADER
#version 460 core

in VS_OUT {
    vec2 uv;
} fs_in;

layout(binding = 2) uniform sampler2D rendered_scene;

out vec4 frag_color;

void main() {
    vec3 gamma_corrected = pow(texture(rendered_scene, fs_in.uv).rgb, vec3(1/2.2));
    frag_color = vec4(gamma_corrected, 1.0);
}