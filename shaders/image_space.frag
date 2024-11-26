//FRAGMENT_SHADER
#version 460 core
in VS_OUT {
    vec2 uv;
} fs_in;

layout(binding = 2) uniform sampler2D scene_model;
layout(binding = 3) uniform sampler2D scene_sky;

const float EXPOSURE = 0.7;

out vec4 frag_color;

void main() {
    vec3 scene_color = texture(scene_model, fs_in.uv).rgb;
    vec3 tone_mapped = vec3(1.0) - exp(-scene_color * EXPOSURE);

    vec3 sky_color = texture(scene_sky, fs_in.uv).rgb;

    vec3 gamma_corrected = pow(tone_mapped + sky_color, vec3(1/2.2));
    frag_color = vec4(gamma_corrected, 1.0);
}