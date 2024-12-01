//VERTEX_SHADER
#version 460 core
#include "ssbo_sky_vertex.glsl"
#include "ubo_matrices.glsl"
out VS_OUT {
    vec4 model_space_position;
} vs_out;

void main() {
    vs_out.model_space_position = vec4(getPosition(gl_VertexID), 1.0);

    mat4 view_rotation = mat4(mat3(view));  // remove translation component
    vec4 clip_space_position = projection * view_rotation * vs_out.model_space_position;

    gl_Position = clip_space_position.xyww; // depth testing trick
}