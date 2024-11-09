#include "skybox.h"
#include "stb_image.h"

#include <format>

Skybox::Skybox(const std::vector<std::string>& paths) {
  /// Create and fill buffer
  glCreateBuffers(1, &vertex_buffer_.id);
  glNamedBufferStorage(vertex_buffer_.id, sizeof(VERTICES), VERTICES, 0);

  /// Create and fill cubemap texture
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cube_map_.id);
  glTextureStorage2D(cube_map_.id, 1, GL_RGB8, FACE_SIZE, FACE_SIZE);

  int width, height, nrComponents;
  for (int i = 0; i < 6; ++i) {
    unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nrComponents, STBI_rgb);
    if (!data) {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1,
                           std::format("(Skybox::Skybox): Failed to read file from path '{}.'", paths[i]).c_str());
    } else if (width != FACE_SIZE || height != FACE_SIZE) {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1,
                           std::format("(Skybox::Skybox): Texture '{}' does not have required dimensions "
                                       "({}x{}).", paths[i], FACE_SIZE, FACE_SIZE).c_str());
    } else if (nrComponents < 3) {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1,
                           std::format("(Skybox::Skybox): Texture '{}' does not have required number of "
                                       "channels (>=3).", paths[i]).c_str());
    } else {
      glTextureSubImage3D(cube_map_.id, 0, 0, 0, i, FACE_SIZE, FACE_SIZE, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    stbi_image_free(data);
  }
  glTextureParameteri(cube_map_.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(cube_map_.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(cube_map_.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(cube_map_.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(cube_map_.id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Skybox::drawSetup(const std::unique_ptr<ShaderProgram>& shader) const {
  shader->use();
  shader->setInt("cubemap", 1);
  glBindTextureUnit(1, cube_map_.id);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertex_buffer_.id);
}
