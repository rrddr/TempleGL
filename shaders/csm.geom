//GEOMETRY_SHADER
#version 460 core
layout(triangles, invocations = 3) in;
layout(triangle_strip, max_vertices = 3) out;

layout (binding = 0, std140) uniform matrix_ubo {
    mat4 projection;
    mat4 view;
    mat4 sunlight_space[3];
};

void main() {
    for (int i = 0; i < 3; ++i) {
        gl_Position = sunlight_space[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}