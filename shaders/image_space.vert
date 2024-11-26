//VERTEX_SHADER
#version 460 core
// By specifying coordinates outside NDC range, we can fill screen with a single triangle instead of two
const vec2[3] vertices = {
    { -1.0, -1.0 },
    { 3.0, -1.0 },
    { -1.0, 3.0 }
};

out VS_OUT {
    vec2 uv;
} vs_out;

void main() {
    vs_out.uv = clamp(vertices[gl_VertexID], vec2(0.0), vec2(2.0));
    gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}