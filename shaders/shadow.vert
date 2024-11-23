//VERTEX_SHADER
#version 460 core
// This should match the definition in src/model.h
struct Vertex {
    float position[3];
    float tangent[3];
    float bitangent[3];
    float uv[2];
};
layout (binding = 0, std430) readonly buffer vertex_ssbo {
    Vertex data[];
};

layout (binding = 0, std140) uniform matrix_ubo {
    mat4 projection;
    mat4 view;
    mat4 sunlight_space;
};

vec3 getPosition(int index);

void main() {
    gl_Position = sunlight_space * vec4(getPosition(gl_VertexID), 1.0);
}

vec3 getPosition(int index) {
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]
    );
}
