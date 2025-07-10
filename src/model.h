#ifndef TEMPLEGL_SRC_MODEL_H_
#define TEMPLEGL_SRC_MODEL_H_

#include "opengl_wrappers.h"
#include "shader_program.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <memory>

/**
 * Implements everything needed to draw a model, using Multi-Draw Indirect and a uniform array for textures.
 */
class Model {
public:
  std::vector<glm::vec4> light_positions_;

  /**
   * Parses a Wavefront .obj file, and loads the data into OpenGL buffers.
   * <p>
   * No .mtl file is needed. Instead, the material name will be used to look for textures at:
   * <folder_path>/diffuse/<mtl-name>.png,
   * <folder_path>/normal/<mtl-name>.png, and
   * <folder_path>/specular/<mtl-name>.png.
   * <p>
   * The textures should be 128x128, in RGB or RGBA format (4th channel will be ignored). Missing textures will be
   * replaced by DefaultMaterial.png (if available), but bad textures may result in unexpected behaviour.
   * <p>
   * Lighting information may be provided in a second .obj file. Each face of each mesh using material "light_source"
   * will be interpreted as a point light (by averaging the vertices). All other meshes will be ignored.
   *
   * @param folder_path Path to a folder (global, or relative to executable) containing a model.obj file, the texture
   *                    folders, and optionally a lights.obj file as described above.
   */
  explicit Model(std::string folder_path);

  /**
   * Binds GL_DRAW_INDIRECT_BUFFER, GL_ELEMENT_ARRAY_BUFFER and GL_SHADER_STORAGE_BUFFER at vertex_buffer_binging.
   * Binds texture_array_ to the texture unit specified by texture_binding.
   */
  void drawSetup(GLuint vertex_buffer_binding, GLuint texture_binding) const;

  /**
   * Draws the model. drawSetup() must have been called at least once before this method.
   *
   * @param shader  Should read vertex data from an SSBO containing an array of Vertex structs matching the definition
   *                below. May define a sampler2DArray uniform. Bindings should equal the ones passed to drawSetup().
   *                The vertex and material indices will be available as gl_VertexID and gl_BaseInstance respectively.
   *                The diffuse texture for a given material is layer gl_BaseInstance*3 of the texture array
   *                (normal is *3+1, specular is *3+2).
   */
  void draw(const std::unique_ptr<ShaderProgram>& shader) const {
    shader->use();
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
    GLuint base_instance; // We don't use instancing, so this is re-purposed as material index of the mesh
  };

  Assimp::Importer importer_ {};
  std::string source_dir_;
  GLsizei num_draw_commands_ {};

  wrap::Buffer vertex_buffer_ {};
  wrap::Buffer index_buffer_ {};
  wrap::Buffer draw_command_buffer_ {};
  wrap::Texture texture_array_ {};

  void loadModelData();
  void loadLightData();
  void createTextureArray(aiMaterial** materials, unsigned int num_materials);
  void createBuffers(aiMesh** meshes, unsigned int num_meshes);
  static void createBufferFromVector(wrap::Buffer& buffer, const std::vector<auto>& vector);
  void checkAssimpSceneErrors(const aiScene* scene, const std::string& path) const;

  static constexpr GLsizei TEX_SIZE {128};
};
#endif //TEMPLEGL_SRC_MODEL_H_
