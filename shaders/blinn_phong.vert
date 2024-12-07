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
    mat4 sunlight_transform[3];
};

out VS_OUT {
    flat int material_index;
    smooth vec2 uv;
    smooth vec4 world_space_position;
    smooth vec4 view_space_position;
    smooth vec4 sunlight_space_position[3];
    flat mat3 TBN;
} vs_out;

vec3 getPosition(int index);
mat3 getTBN(int index);
vec2 getUV(int index);

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

vec3 getPosition(int index) {
    return vec3(data[index].position[0],
                data[index].position[1],
                data[index].position[2]);
}

mat3 getTBN(int index) {
    vec3 T = vec3(data[index].tangent[0],
                  data[index].tangent[1],
                  data[index].tangent[2]);
    vec3 B = vec3(data[index].bitangent[0],
                  data[index].bitangent[1],
                  data[index].bitangent[2]);
    vec3 N = cross(T, B);
    return mat3(T, B, N);
}

vec2 getUV(int index) {
    return vec2(data[index].uv[0],
                data[index].uv[1]);
}
