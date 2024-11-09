#include "stbi_helpers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glad/glad.h>

#include <format>
#include <memory>

void help::StbiDeleter::operator()(unsigned char* data) {
  stbi_image_free(data);
}

void help::fill3DTextureLayer(const std::string& path, const wrap::Texture& texture, int layer, int width, int height) {
  int actual_width, actual_height, actual_num_components;
  auto data = std::unique_ptr<unsigned char, StbiDeleter>(stbi_load(path.c_str(),
                                                                    &actual_width,
                                                                    &actual_height,
                                                                    &actual_num_components,
                                                                    STBI_rgb),
                                                          StbiDeleter());
  if (!data) {
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1,
                         std::format("(help::fill3DTextureLayer): Failed to read file from path '{}.'",
                                     path).c_str());
  } else if (actual_width != width || actual_height != height) {
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1,
                         std::format("(help::fill3DTextureLayer): Texture '{}' does not have required dimensions "
                                     "({}x{}).", path, width, height).c_str());
  } else if (actual_num_components < 3) {
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1,
                         std::format("(help::fill3DTextureLayer): Texture '{}' does not have required number of "
                                     "channels (>=3).", path).c_str());
  } else {
    glTextureSubImage3D(texture.id, 0, 0, 0, layer, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data.get());
  }
}
