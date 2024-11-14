#ifndef TEMPLEGL_SRC_SKYBOX_H_
#define TEMPLEGL_SRC_SKYBOX_H_

#include "opengl_wrappers.h"
#include "shader_program.h"

#include <vector>
#include <string>
#include <memory>

class Skybox {
 public:
  /**
   * Creates required OpenGL resources.
   *
   * @param paths   List of paths to the 6 faces of a cube map, in +X, -X, +Y, -Y, +Z, -Z order. The textures should be
   *                512x512, in RGB or RGBA format (4th channel will be ignored).
   */
  explicit Skybox(const std::vector<std::string>& paths);

  /**
   * Binds GL_SHADER_STORAGE_BUFFER at vertex_buffer_binding. Binds cube_map_ to unit specified by texture_binding.
   */
  void drawSetup(GLuint vertex_buffer_binding,
                 GLuint texture_binding) const;
  /**
   * Draws the skybox. drawSetup() must have been called at least once before this method.
   *
   * @param shader  Should read vertex data from an SSBO containing a single array of 108 floats (coordinates in
   *                x1, y1, z1, x2, y2, z2... order). Should define a samplerCube uniform. Bindings should equal the
   *                ones passed to drawSetup().
   */
  static inline void draw(const std::unique_ptr<ShaderProgram>& shader) {
    shader->use();
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

 private:
  wrap::Texture cube_map_ {};
  wrap::Buffer vertex_buffer_ {};
  static constexpr GLsizei FACE_SIZE {512};
  static constexpr GLfloat VERTICES[108] {
      -0.5f, -0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      0.5f, 0.5f, -0.5f,
      0.5f, 0.5f, -0.5f,
      -0.5f, 0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f,

      -0.5f, -0.5f, 0.5f,
      0.5f, -0.5f, 0.5f,
      0.5f, 0.5f, 0.5f,
      0.5f, 0.5f, 0.5f,
      -0.5f, 0.5f, 0.5f,
      -0.5f, -0.5f, 0.5f,

      -0.5f, 0.5f, 0.5f,
      -0.5f, 0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f, 0.5f,
      -0.5f, 0.5f, 0.5f,

      0.5f, 0.5f, 0.5f,
      0.5f, 0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      0.5f, -0.5f, 0.5f,
      0.5f, 0.5f, 0.5f,

      -0.5f, -0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      0.5f, -0.5f, 0.5f,
      0.5f, -0.5f, 0.5f,
      -0.5f, -0.5f, 0.5f,
      -0.5f, -0.5f, -0.5f,

      -0.5f, 0.5f, -0.5f,
      0.5f, 0.5f, -0.5f,
      0.5f, 0.5f, 0.5f,
      0.5f, 0.5f, 0.5f,
      -0.5f, 0.5f, 0.5f,
      -0.5f, 0.5f, -0.5f,
  };
};
#endif //TEMPLEGL_SRC_SKYBOX_H_
