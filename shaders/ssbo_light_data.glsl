//INCLUDE_TARGET
struct CameraParameters {
    vec4 world_space_position;
    float csm_partition_depths[3];
};
struct Light {
    vec4 source;
    vec4 color;
    float intensity;
};
layout (binding = SSBO_LIGHT_DATA, std430) readonly buffer light_data_ssbo {
    CameraParameters camera;
    Light sunlight;
    uint num_point_lights;
    Light point_lights[];
};
