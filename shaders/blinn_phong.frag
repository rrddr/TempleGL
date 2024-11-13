//FRAGMENT_SHADER
#version 460 core
in VS_OUT {
    flat int material_index;
    smooth vec2 uv;
    smooth vec4 position;
    flat mat3 TBN;
} fs_in;

struct CameraParameters {
    vec4 position;
};
struct DirectionalLight {
    vec4 origin; // pointing toward "source"
    vec4 color;
};
layout (binding = 2, std430) readonly buffer light_ssbo {
    CameraParameters camera;
    DirectionalLight sunlight;
};

uniform sampler2DArray texture_array;

const float SPECULAR_EXPONENT = 16.0;
const vec3 AMBIENT_LIGHT = vec3(0.3);

out vec4 frag_color;

vec3 calculateSunlightContribution(vec3 diffuse_color, vec3 N, vec3 V, float specular_factor);

void main() {
    // Retrieve values from texture array
    vec3 raw_diffuse = texture(texture_array, vec3(fs_in.uv, float(fs_in.material_index * 3))).rgb;
    vec3 raw_normal = texture(texture_array, vec3(fs_in.uv, float(fs_in.material_index * 3 + 1))).xyz;
    float raw_specular = texture(texture_array, vec3(fs_in.uv, float(fs_in.material_index * 3 + 2))).r;

    // Compute intermediates
    vec3 diffuse_color = pow(raw_diffuse, vec3(2.2));
    float specular_factor = raw_specular; // TODO: tweak
    vec3 N = fs_in.TBN * normalize(raw_normal * 2.0 - 1.0);
    vec3 V = normalize((camera.position - fs_in.position).xyz);

    // Compute contributions from light sources
    vec3 sunlight_contribution = calculateSunlightContribution(diffuse_color, N, V, specular_factor);

    frag_color = vec4(AMBIENT_LIGHT*diffuse_color + sunlight_contribution, 1.0);
}

vec3 calculateSunlightContribution(vec3 diffuse_color, vec3 N, vec3 V, float specular_factor) {
    vec3 L = normalize(sunlight.origin.xyz);
    vec3 diffuse = diffuse_color * sunlight.color.rgb * max(dot(N, L), 0.0);

    vec3 H = normalize(L + V); // halfway vector
    vec3 specular = min(specular_factor * pow(max(dot(N, H), 0.0), SPECULAR_EXPONENT), 0.4) * sunlight.color.rgb;

    return diffuse + specular;
}
