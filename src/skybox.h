#ifndef TEMPLEGL_SRC_SKYBOX_H_
#define TEMPLEGL_SRC_SKYBOX_H_

#include "opengl_wrappers.h"
#include "shader_program.h"

#include <vector>
#include <string>
#include <memory>

/**
 * Draws the skybox.
 */
class Skybox {
 public:
  /**
   * Creates required OpenGL resources.
   *
   * @param paths   List of paths to the 6 faces of a cubemap, in +X, -X, +Y, -Y, +Z, -Z order. The textures should be
   *                512x512, in RGB or RGBA format (4th channel will be ignored).
   */
  explicit Skybox(const std::vector<std::string>& paths);

  /**
   * Binds GL_SHADER_STORAGE_BUFFER at binding point 1. Binds cubemap texture to Texture Unit 1 and sets the
   * corresponding shader uniform.
   * <p>
   * Must be called before the first call to draw(), and again if any of the above are rebound, or a different
   * shader is to be used.
   *
   * @param shader  Should read vertex data from an SSBO with binding = 1, containing  a single array of 108 floats
   *                (coordinates in x1, y1, z1, x2, y2, z2... order). Should define a samplerCube uniform named
   *                "cubemap".
   */
  void drawSetup(const std::unique_ptr<ShaderProgram>& shader) const;
  inline void draw() const {
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

 private:
  wrap::Texture cube_map_ {};
  wrap::Buffer vertex_buffer_ {};
  static constexpr GLsizei FACE_SIZE {512};
  static constexpr GLfloat VERTICES[] {
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
