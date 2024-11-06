#ifndef TEMPLEGL_SRC_MODEL_H_
#define TEMPLEGL_SRC_MODEL_H_

#include "opengl_wrappers.h"
#include "shader_program.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>
#include <memory>

/**
 *
 */
class Model {
 public:
  explicit Model(const std::string& obj_path);

  void drawSetup(const std::unique_ptr<ShaderProgram>& shader) const;
  inline void draw() const {
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, num_draw_commands_, 0);
  }

 private:
  struct Vertex {
    GLfloat position[3];
    GLfloat uv[2];
  };
  struct DrawElementsIndirectCommand {
    GLuint count;
    GLuint instance_count;
    GLuint first_vertex;
    GLint  base_vertex;
    GLuint base_instance;
  };

  std::string source_dir_;
  GLsizei num_draw_commands_;

  wrap::Buffer vertex_buffer_ {};
  wrap::Buffer index_buffer_ {};
  wrap::Buffer draw_command_buffer_ {};
  wrap::Texture texture_array_ {};

  template<typename T>
  static void createBufferFromVector(wrap::Buffer& buffer, std::vector<T> vector);
  void createBuffers(aiMesh** meshes, unsigned int num_meshes);
  void createTextureArray(aiMaterial** materials, unsigned int num_materials);
};
#endif //TEMPLEGL_SRC_MODEL_H_