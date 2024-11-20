//FRAGMENT_SHADER
#version 460 core

in VS_OUT {
    vec2 uv;
} fs_in;

layout(binding = 2) uniform sampler2D rendered_scene;
layout(binding = 3) uniform sampler2D rendered_sky;

const float EXPOSURE = 0.5;

out vec4 frag_color;

void main() {
    vec3 scene_color = texture(rendered_scene, fs_in.uv).rgb;
    vec3 tone_mapped = vec3(1.0) - exp(-scene_color * EXPOSURE);

    vec3 sky_color = texture(rendered_sky, fs_in.uv).rgb;

    vec3 gamma_corrected = pow(tone_mapped + sky_color, vec3(1/2.2));
    frag_color = vec4(gamma_corrected, 1.0);
}