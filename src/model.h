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
 * Implements everything needed to draw a model, using Multi-Draw Indirect and a uniform array for textures.
 */
class Model {
 public:
  /**
   * Parses a Wavefront .obj file, and loads the data into OpenGL buffers.
   * <p>
   * No .mtl file is needed. Instead the material name will be used to look for textures at:
   * <.obj-parent-dir>/diffuse/<mtl-name>.png,
   * <.obj-parent-dir>/normal/<mtl-name>.png, and
   * <.obj-parent-dir>/specular/<mtl-name>.png.
   * <p>
   * The textures should be 128x128, in RGB or RGBA format (4th channel will be ignored). Missing textures will be
   * replaced by DefaultMaterial.png (if available), but bad textures may result in unexpected behaviour.
   *
   * @param obj_path    Path to a Wavefront .obj file satisfying the above requirements.
   */
  explicit Model(const std::string& obj_path);

  /**
   * Binds GL_DRAW_INDIRECT_BUFFER, GL_ELEMENT_ARRAY_BUFFER and GL_SHADER_STORAGE_BUFFER at binding point 0.
   * Binds texture array to Texture Unit 0 and sets the corresponding shader uniform.
   * <p>
   * Must be called before the first call to draw(), and again if any of the above are rebound, or a different
   * shader is to be used.
   *
   * @param shader  Should read vertex data from an SSBO with binding = 0, containing an array of Vertex structs
   *                matching the definition below. Should define a sampler2DArray uniform named "texture_array".
   *                The vertex and material indices will be available as gl_VertexID and gl_BaseInstance respectively.
   *                The diffuse texture for a given material is layer index*3 of texture_array (normal is *3+1,
   *                specular is *3+2).
   */
  void drawSetup(const std::unique_ptr<ShaderProgram>& shader) const;
  inline void draw() const {
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, num_draw_commands_, 0);
  }

 private:
  struct Vertex {
    GLfloat position[3];
    GLfloat tangent[3];
    GLfloat bitangent[3];
    GLfloat uv[2];
  };
  struct DrawElementsIndirectCommand {
    GLuint count;
    GLuint instance_count;
    GLuint first_vertex;
    GLint base_vertex;
    GLuint base_instance;   // We don't use instancing, so this is re-purposed as the material index of the mesh
  };

  std::string source_dir_;
  GLsizei num_draw_commands_;

  wrap::Buffer vertex_buffer_ {};
  wrap::Buffer index_buffer_ {};
  wrap::Buffer draw_command_buffer_ {};
  wrap::Texture texture_array_ {};

  static constexpr GLsizei TEX_SIZE {128};

  template<typename T>
  static void createBufferFromVector(wrap::Buffer& buffer, const std::vector<T>& vector);
  void createBuffers(aiMesh** meshes, unsigned int num_meshes);
  void createTextureArray(aiMaterial** materials, unsigned int num_materials);
};
#endif //TEMPLEGL_SRC_MODEL_H_