//VERTEX_SHADER
#version 460 core
#include "ssbo_temple_vertex.glsl"
#include "ubo_matrices.glsl"

out VS_OUT {
    flat int material_index;
    smooth vec2 uv;
    smooth vec4 world_space_position;
    smooth vec4 view_space_position;
    smooth vec4 sunlight_space_position[3];
    flat mat3 TBN;
} vs_out;

void main() {
    vs_out.material_index = gl_BaseInstance;
    vs_out.uv = getUV(gl_VertexID);
    vs_out.world_space_position = vec4(getPosition(gl_VertexID), 1.0);
    vs_out.view_space_position = view * vs_out.world_space_position;
    for (int i = 0; i < 3; ++i) {
        vs_out.sunlight_space_position[i] = sunlight_transform[i] * vs_out.world_space_position;
    }
    vs_out.TBN = getTBN(gl_VertexID);

    gl_Position = projection * vs_out.view_space_position;
}