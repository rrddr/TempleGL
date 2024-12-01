//VERTEX_SHADER
#version 460 core
#include "ssbo_light_data.glsl"
#include "ubo_matrices.glsl"

void main() {
    gl_Position = projection * view * point_lights[gl_VertexID].source;
}