#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "helper.h"

#include <iostream>

/// Global Variables
int window_height = 1080;
int window_width = 1920;

int main() {
  /// Initialize GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
  std::cout << "Running in Debug Mode" << std::endl;
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

  GLFWwindow* window = glfwCreateWindow(window_width, window_height, "TempleGL", nullptr, nullptr);
  if (window == nullptr) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//  glfwSetCursorPosCallback(window, mouse_callback);
//  glfwSetScrollCallback(window, scroll_callback);

  glfwSetWindowPos(window, 50, 50); // TODO: Make portable?

  /// Initialize GLAD (which loads OpenGL function pointers for us)
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
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
