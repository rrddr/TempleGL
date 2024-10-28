#include "screen.h"

#include <iostream>

void ScreenExternal::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
  auto screen = reinterpret_cast<Screen*>(glfwGetWindowUserPointer(window));
  screen->internalFramebufferSizeCallback(width, height);
}

void ScreenExternal::mouseCallback(GLFWwindow* window, double x_pos, double y_pos) {
  auto screen = reinterpret_cast<Screen*>(glfwGetWindowUserPointer(window));
  screen->internalMouseCallback((float) x_pos, (float) y_pos);
}

void ScreenExternal::scrollCallback(GLFWwindow* window, double x_offset, double y_offset) {
  auto screen = reinterpret_cast<Screen*>(glfwGetWindowUserPointer(window));
  screen->internalScrollCallback((float) y_offset);
}

Screen::Screen(int width, int height, const std::string& window_name, const Camera& camera_ref, bool debug_mode)
    : window_width(width), window_height(height), camera(std::make_unique<Camera>(camera_ref)) {
  /// Initialize GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  if (debug_mode) {
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  }

  /// Create GLFW window
  glfw_window = glfwCreateWindow(window_width, window_height, window_name.c_str(), nullptr, nullptr);
  if (glfw_window == nullptr) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window object.");
  }
  glfwMakeContextCurrent(glfw_window);
  glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetWindowPos(glfw_window, 50, 50);

  /// Register GLFW callbacks
  // WindowUserPointer allows us to reference this class instance from static callback functions
  glfwSetWindowUserPointer(glfw_window, reinterpret_cast<void*>(this));
  glfwSetFramebufferSizeCallback(glfw_window, ScreenExternal::framebufferSizeCallback);
  glfwSetCursorPosCallback(glfw_window, ScreenExternal::mouseCallback);
  glfwSetScrollCallback(glfw_window, ScreenExternal::scrollCallback);

  /// Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    glfwTerminate();
    throw std::runtime_error("Failed to initialize GLAD.");
  }

  /// OpenGL options
  glEnable(GL_DEPTH_TEST);

  /// Set remaining variables
  current_time = (float) glfwGetTime();
  // delta_time, mouse_x, mouse_y   These will be initialized on first use
}

void Screen::processKeyboardInput() {
  /// Update time variables
  auto new_time = (float) glfwGetTime();
  delta_time = new_time - current_time;
  current_time = new_time;

  /// Process keyboard input
  if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(glfw_window, true);
  if (glfwGetKey(glfw_window, GLFW_KEY_W) == GLFW_PRESS)
    camera->processKeyboard(FORWARD, delta_time);
  if (glfwGetKey(glfw_window, GLFW_KEY_S) == GLFW_PRESS)
    camera->processKeyboard(BACKWARD, delta_time);
  if (glfwGetKey(glfw_window, GLFW_KEY_A) == GLFW_PRESS)
    camera->processKeyboard(LEFT, delta_time);
  if (glfwGetKey(glfw_window, GLFW_KEY_D) == GLFW_PRESS)
    camera->processKeyboard(RIGHT, delta_time);
  if (glfwGetKey(glfw_window, GLFW_KEY_SPACE) == GLFW_PRESS)
    camera->processKeyboard(UP, delta_time);
  if (glfwGetKey(glfw_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    camera->processKeyboard(DOWN, delta_time);
}

void Screen::internalFramebufferSizeCallback(int width, int height) {
  window_width = width;
  window_height = height;
  if (!lockGlViewport) glViewport(0, 0, width, height);
}

void Screen::internalMouseCallback(float x_pos, float y_pos) {
  // This prevents a large camera jump on start
  if (first_time_receiving_mouse_input) {
    first_time_receiving_mouse_input = false;
  } else {
    camera->processMouseMovement(x_pos - mouse_x, mouse_y - y_pos);
  }
  mouse_x = x_pos;
  mouse_y = y_pos;
}

void Screen::internalScrollCallback(float y_offset) const {
  camera->processMouseScroll(y_offset, delta_time);
}