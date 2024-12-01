//VERTEX_SHADER
#version 460 core
#include "ssbo_temple_vertex.glsl"

void main() {
    gl_Position = vec4(getPosition(gl_VertexID), 1.0);
}