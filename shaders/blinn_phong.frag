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
struct Light {
    vec4 source;
    vec4 color;
    float intensity;
};
layout (binding = 2, std430) readonly buffer light_ssbo {
    CameraParameters camera;
    Light sunlight;
    uint num_point_lights;
    Light point_lights[];
};

layout (binding = 0) uniform sampler2DArray texture_array;

const float SPECULAR_EXPONENT = 16.0;
const float POINT_LIGHT_MAX_R = 7.0;
const vec3 AMBIENT_LIGHT = vec3(1.0);

out vec4 frag_color;

vec3 calculateBlinnPhong(vec3 L, vec3 N, vec3 V, vec3 diffuse_color, vec3 light_color, float specular_factor);
float calculateAttenuation(float intensity, float source_distance);

void main() {
    // Retrieve values from texture array
    vec3 raw_diffuse = texture(texture_array, vec3(fs_in.uv, float(fs_in.material_index * 3))).rgb;
    vec3 raw_normal = texture(texture_array, vec3(fs_in.uv, float(fs_in.material_index * 3 + 1))).xyz;
    float raw_specular = texture(texture_array, vec3(fs_in.uv, float(fs_in.material_index * 3 + 2))).r;

    // Compute intermediates
    vec3 diffuse_color = pow(raw_diffuse, vec3(2.2));
    float specular_factor = 1.0 - pow(1.0 - raw_specular, 2.0);
    vec3 N = fs_in.TBN * normalize(raw_normal * 2.0 - 1.0);
    vec3 V = normalize(camera.position.xyz - fs_in.position.xyz);

    // Compute contributions from light sources
    vec3 sunlight_contribution = sunlight.intensity * calculateBlinnPhong(
        normalize(sunlight.source.xyz), N, V, diffuse_color, sunlight.color.rgb, specular_factor
    );
    vec3 point_light_contribution = vec3(0.0);
    for (int i = 0; i < num_point_lights; ++i) {
        vec3 relative_light_position = point_lights[i].source.xyz - fs_in.position.xyz;
        float attenuation = calculateAttenuation(point_lights[i].intensity, length(relative_light_position));
        point_light_contribution += attenuation * calculateBlinnPhong(
            normalize(relative_light_position), N, V, diffuse_color, point_lights[i].color.rgb, specular_factor
        );
    }

    frag_color = vec4(AMBIENT_LIGHT * diffuse_color + sunlight_contribution + point_light_contribution, 1.0);
}

vec3 calculateBlinnPhong(vec3 L, vec3 N, vec3 V, vec3 diffuse_color, vec3 light_color, float specular_factor) {
    vec3 diffuse = light_color * max(dot(N, L), 0.0);
    vec3 H = normalize(L + V); // halfway vector
    vec3 specular = specular_factor * light_color * pow(max(dot(N, H), 0.0), SPECULAR_EXPONENT);

    return diffuse_color * (diffuse + specular);
}

float calculateAttenuation(float intensity, float source_distance) {
    return intensity * pow(max(1.0 - pow(source_distance / POINT_LIGHT_MAX_R, 4.0), 0.0), 2.0)
        * (1.0 / (pow(source_distance, 2.0), 0.1));
}
