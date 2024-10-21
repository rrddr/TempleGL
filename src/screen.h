#ifndef TEMPLEGL_SRC_SCREEN_H_
#define TEMPLEGL_SRC_SCREEN_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

#include <string>
#include <memory>

/**
 * Callback functions for GLFW, which simply reference their internal counterparts in the Screen class
 */
namespace ScreenExternal {
  void framebufferSizeCallback(GLFWwindow* window, int width, int height);
  void mouseCallback(GLFWwindow* window, double x_pos, double y_pos);
  void scrollCallback(GLFWwindow* window, double x_offset, double y_offset);
}

/**
 * This class collects GLFW/GLAD boilerplate code, rendering-relevant variables that would otherwise be global,
 * and a Camera instance.
 */
class Screen {
 public:
  bool lockGlViewport = false;      // use this to prevent calls to glViewport from this class when rendering to an FBO
  GLFWwindow* glfw_window;          // raw pointer, as destruction is handled by glfwTerminate()
  std::unique_ptr<Camera> camera;

  /**
   * Constructor calls glfwInit(), creates the GLFWwindow object, and initializes GLAD.
   * Avoid creating multiple instances of this class.
   *
   * @param width       Desired widow width in pixels.
   * @param height      Desired window height in pixels.
   * @param window_name Name of window (displayed in window header).
   * @param camera      A Camera instance (this class will take ownership of the object).
   * @param debug_mode  If true, GLFW will create a debug context.
   *
   * @throws runtime_error  If initialization of either GLFW window or GLAD fails.
   */
  Screen(int width, int height, const std::string& window_name, const Camera& camera, bool debug_mode = false);
  ~Screen() { glfwTerminate(); }

  /**
   * Computes transform matrix from view space to clip space.
   *
   * @param fov         Vertical field of view in radians (angle between top and bottom planes of view frustrum).
   * @param near_plane  Distance of near plane from camera.
   * @param far_plane   Distance of far plane from camera.
   */
  inline glm::mat4 getProjectionMatrix(float fov = HALF_PI, float near_plane = 0.1f, float far_plane = 100.0f) const {
    return glm::perspective(fov, (float) window_width / (float) window_height, near_plane, far_plane);
  }

  // These methods handle all user interaction
  void processKeyboardInput();
  void internalFramebufferSizeCallback(int width, int height);
  void internalMouseCallback(float x_pos, float y_pos);
  void internalScrollCallback(float y_offset) const;

 private:
  bool first_time_receiving_mouse_input = true;
  float mouse_x, mouse_y, current_time, delta_time;
  int window_width, window_height;
};

#endif //TEMPLEGL_SRC_SCREEN_H_
