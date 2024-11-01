#ifndef TEMPLEGL_SRC_INITIALIZER_H_
#define TEMPLEGL_SRC_INITIALIZER_H_

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <string>

enum DebugLevel { ALL, LOW, MEDIUM, HIGH };
struct MinimalConfig {
  std::string window_name;
  int window_width;
  int window_height;
  int window_initial_x_pos;
  int window_initial_y_pos;
  bool debug_enabled;
  DebugLevel debug_level;
};

/**
 * Defines the abstract structure of the program. This class implements only boilerplate, and provides dummy methods
 * that should be overridden to provide actual functionality.
 *
 * @tparam CONFIG_TYPE  Should derive from MinimalConfig.
 */
template <typename CONFIG_TYPE>
class Initializer {
 public:
  virtual void run() final;

 protected:
  CONFIG_TYPE config_{};
  GLFWwindow* window_{};        // raw pointer, as destruction is handled by glfwTerminate()
  bool lock_gl_viewport{false}; // prevent unwanted calls to glViewport while rendering to a non-default framebuffer

  /// Program stages
  virtual void loadConfigYaml();
  virtual void init() final;
  virtual void renderSetup() {}
  virtual void updateRenderState() {}
  virtual void processKeyboardInput();  // Default implementation terminates program on ESC press
  virtual void render() {}
  virtual void renderTerminate() {}

  /// Callbacks
  virtual void framebufferSizeCallback(int width, int height) final;
  virtual void cursorPosCallback(float x_pos, float y_pos) {}
  virtual void scrollCallback(float y_offset) {}
  static void glfwErrorCallback(int error_code, const char* description);
  static void APIENTRY debugMessageCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                            GLsizei length, const char* message, const void* user_param);
};
#endif //TEMPLEGL_SRC_INITIALIZER_H_
