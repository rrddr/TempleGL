//GEOMETRY_SHADER
#version 460 core
#include "ubo_matrices.glsl"

layout(triangles, invocations = 3) in;
layout(triangle_strip, max_vertices = 3) out;

void main() {
    for (int i = 0; i < 3; ++i) {
        gl_Position = sunlight_transform[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}