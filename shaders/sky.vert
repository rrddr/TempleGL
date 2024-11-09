//VERTEX_SHADER
#version 460 core
layout (binding = 0, std140) uniform matrix_ubo {
    uniform mat4 projection;
    uniform mat4 view;
};

layout (binding = 1, std430) readonly buffer vertex_ubo {
    float data[108]; // This should match the hardcoded data in src/skybox.h
};

out VS_OUT {
    vec3 model_space_position;
} vs_out;

vec3 getPosition(int index);

void main() {
    vs_out.model_space_position = getPosition(gl_VertexID);

    mat4 view_rotation = mat4(mat3(view));  // remove translation component
    vec4 clip_space_position = projection * view_rotation * vec4(vs_out.model_space_position, 1.0);

    gl_Position = clip_space_position.xyww; // depth testing trick
}

vec3 getPosition(int index) {
    return vec3(
        data[index * 3],
        data[index * 3 + 1],
        data[index * 3 + 2]
    );
}
