//INCLUDE_TARGET
// This should match the definition in src/model.h
struct Vertex {
    float position[3];
    float tangent[3];
    float bitangent[3];
    float uv[2];
};
layout (binding = 0, std430) readonly buffer temple_vertex_ssbo {
    Vertex data[];
};

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