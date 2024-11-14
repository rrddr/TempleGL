//VERTEX_SHADER
#version 460 core

const vec2[4] vertices = {
    {-1.0, -1.0},
    {1.0, -1.0},
    {-1.0, 1.0},
    {1.0, 1.0}
};

out VS_OUT {
    vec2 uv;
} vs_out;

void main() {
    vs_out.uv = max(vertices[gl_VertexID], vec2(0.0));
    gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}