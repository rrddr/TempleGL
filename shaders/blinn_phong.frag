//FRAGMENT_SHADER
#version 460 core
in VS_OUT {
    flat int material_index;
    smooth vec2 uv;
    smooth vec4 position;
    smooth vec4 view_space_position;
    smooth vec4 sunlight_space_position[3];
    flat mat3 TBN;
} fs_in;

struct CameraParameters {
    vec4 position;
    float csm_partition_depths[3];
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
layout (binding = 4) uniform sampler2DArray sunlight_shadow_map;

const float SPECULAR_EXPONENT = 16.0;
const float POINT_LIGHT_MAX_R = 7.0;
const vec3 AMBIENT_LIGHT = vec3(1.0);

out vec4 frag_color;

float calculateShadow();
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
    vec3 final_color = AMBIENT_LIGHT * diffuse_color;

    final_color += calculateShadow() * sunlight.intensity * calculateBlinnPhong(
        normalize(sunlight.source.xyz), N, V, diffuse_color, sunlight.color.rgb, specular_factor
    );
    for (int i = 0; i < num_point_lights; ++i) {
        vec3 relative_light_position = point_lights[i].source.xyz - fs_in.position.xyz;
        float attenuation = calculateAttenuation(point_lights[i].intensity, length(relative_light_position));
        final_color += attenuation * calculateBlinnPhong(
            normalize(relative_light_position), N, V, diffuse_color, point_lights[i].color.rgb, specular_factor
        );
    }

    frag_color = vec4(final_color, 1.0);
//    frag_color = vec4(calculateShadow(), 0.0, 0.0, 1.0);
//    float layer = calculateShadow();
//    if (layer == 0) {
//        frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
//    }
//    else if (layer == 1) {
//        frag_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
//    }
//    else if (layer == 2) {
//        frag_color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
//    }
}

float calculateShadow() {
    /// Determine cascade layer
    int layer = 2;
    for (int i = 1; i >= 0; --i) {
        if (abs(fs_in.view_space_position.z) < camera.csm_partition_depths[i]) {
            layer = i;
        } else {
            break;
        }
    }
//    return float(layer);

    vec3 remapped_position = (fs_in.sunlight_space_position[layer].xyz / fs_in.sunlight_space_position[layer].w) * 0.5 + 0.5;
    float bias = 0.0005 / (camera.csm_partition_depths[layer] * 0.5);

    float sample_depth = texture(sunlight_shadow_map, vec3(remapped_position.xy, layer)).r;
    return remapped_position.z - bias > sample_depth ? 0.0 : 1.0;
//    // sample shadow map in 3x3 square around actual uv
//    float shadow = 0.0;
//    vec2 texel_size = 1.0 / textureSize(sunlight_shadow_map, 0);
//    for (int x = -1; x <= 1; ++x) {
//        for (int y = -1; y <= 1; ++y) {
//            float sample_depth = texture(sunlight_shadow_map, remapped_position.xy + vec2(x, y) * texel_size).r;
//            shadow += remapped_position.z - bias > sample_depth ? 1.0 : 0.0;
//        }
//    }
//    return 1.0 - shadow / 9.0;
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
