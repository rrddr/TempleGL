//VERTEX_SHADER
#version 460 core
struct CameraParameters {
    vec4 world_space_position;
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
layout (binding = 0, std140) uniform matrix_ubo {
    mat4 projection;
    mat4 view;
    mat4 sunlight_transform[3];
};

void main() {
    gl_Position = projection * view * point_lights[gl_VertexID].source;
}