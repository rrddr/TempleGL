#include "skybox.h"
#include "stbi_helpers.h"

Skybox::Skybox(const std::vector<std::string>& paths) {
  /// Create and fill buffer
  glCreateBuffers(1, &vertex_buffer_.id);
  glNamedBufferStorage(vertex_buffer_.id, sizeof(VERTICES), VERTICES, 0);

  /// Create, fill, and configure cubemap texture
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cube_map_.id);
  glTextureStorage2D(cube_map_.id, 1, GL_RGB8, FACE_SIZE, FACE_SIZE);
  for (int i = 0; i < 6; ++i) {
    help::fill3DTextureLayer(paths[i], cube_map_, i, FACE_SIZE, FACE_SIZE);
  }
  glTextureParameteri(cube_map_.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(cube_map_.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(cube_map_.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(cube_map_.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(cube_map_.id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Skybox::drawSetup(const std::unique_ptr<ShaderProgram>& shader,
                       GLuint texture_unit_id,
                       GLuint vertex_buffer_binding) const {
  shader->use();
  shader->setInt("cubemap", texture_unit_id);
  glBindTextureUnit(texture_unit_id, cube_map_.id);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, vertex_buffer_binding, vertex_buffer_.id);
}
