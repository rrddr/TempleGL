#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "helper.h"
#include "camera.h"
#include "screen.h"

#include <iostream>
#include <memory>

int main() {
  /// Initialize GLFW / GLAD
  bool debug_mode = false;
#ifndef NDEBUG
  std::cout << "Running in Debug Mode" << std::endl;
  debug_mode = true;
#endif
  Camera camera(glm::vec3(0.0f, 10.0f, 0.0f));
  std::unique_ptr<Screen> screen;
  try {
    screen = std::make_unique<Screen>(1080, 1920, "TempleGL", camera, debug_mode);
  }
  catch (std::runtime_error& error) {
    std::cout << "FATAL ERROR: " << error.what() << std::endl;
    glfwTerminate();
    return -1;
  }

  /// Configure OpenGL debug output
  int flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) { // Check if GLFW created a debug context
    std::cout << "OpenGL Debug Output enabled" << std::endl;
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE); // Output all messages
  }

  glfwTerminate();
  return 0;
}
