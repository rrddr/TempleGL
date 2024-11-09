//VERTEX_SHADER
#version 460 core
layout (binding = 0, std140) uniform matrix_ubo {
    uniform mat4 projection;
    uniform mat4 view;
};

// This should match the definition in src/model.h
struct Vertex {
    float position[3];
    float uv[2];
};
layout (binding = 0, std430) readonly buffer vertex_ssbo {
    Vertex data[];
};

out VS_OUT {
    flat int material_index;
    vec2 uv;
} vs_out;

vec3 getPosition(int index);
vec2 getUV(int index);

void main() {
    gl_Position = projection * view * vec4(getPosition(gl_VertexID), 1.0);

    vs_out.material_index = gl_BaseInstance;
    vs_out.uv = getUV(gl_VertexID);
}

vec3 getPosition(int index) {
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]
    );
}

vec2 getUV(int index) {
    return vec2(
        data[index].uv[0],
        data[index].uv[1]
    );
}
