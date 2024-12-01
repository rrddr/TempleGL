//INCLUDE_TARGET
// This should match the hardcoded data in src/skybox.h
layout (binding = 1, std430) readonly buffer sky_vertex_ssbo {
    float data[108];
};

vec3 getPosition(int index) {
    return vec3(data[index * 3],
                data[index * 3 + 1],
                data[index * 3 + 2]);
}