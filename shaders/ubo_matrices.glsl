//INCLUDE_TARGET
layout (binding = UBO_MATRIX, std140) uniform matrix_ubo {
    mat4 projection;
    mat4 view;
    mat4 sunlight_transform[3];
};