#include "initializer.h"

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

#include <iostream>

void Initializer::run() {
  loadConfigYaml();
  init();
  renderSetup();
  while (!glfwWindowShouldClose(window_)) {
    updateRenderState();
    processKeyboardInput();
    render();
    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
  renderTerminate();
  glfwTerminate();
}

void Initializer::loadConfigYaml() {
  YAML::Node config_yaml;
  try {
    config_yaml = YAML::LoadFile("../config.yaml");
  }
  catch (YAML::ParserException& exception) {
    std::cerr << "Initialization ERROR: config.yaml is malformed." << std::endl << exception.what() << std::endl;
    throw std::runtime_error(""); // exit to main() and return -1
  }
  catch (YAML::BadFile& exception) {
    std::cerr << "Initialization ERROR: failed to load config.yaml." << std::endl << exception.what() << std::endl;
    throw std::runtime_error(""); // exit to main() and return -1
  }
  YAML::Node window = config_yaml["window"];
  config_.window_name = window["name"].as<std::string>();
  config_.window_width = window["width"].as<int>();
  config_.window_height = window["height"].as<int>();
  config_.window_initial_x_pos = window["initial_x_pos"].as<int>();
  config_.window_initial_y_pos = window["initial_y_pos"].as<int>();
  YAML::Node debug = config_yaml["debug"];
  config_.debug_enabled = debug["enabled"].as<bool>();
  auto debug_level_str = debug["level"].as<std::string>();
  if (debug_level_str == "all") { config_.debug_level = ALL; }
  else if (debug_level_str == "low") { config_.debug_level = LOW; }
  else if (debug_level_str == "medium") { config_.debug_level = MEDIUM; }
  else if (debug_level_str == "high") { config_.debug_level = HIGH; }
  else {
    std::cerr << "Initialization WARNING: invalid setting in config.yaml, "
              << "debug.level must be one of 'all', 'low', 'medium', 'high'. Defaulting to 'all'.";
    config_.debug_level = ALL;
  }
}

void Initializer::init() {
  /// Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Initialization ERROR: failed to initialize GLFW." << std::endl;
    throw std::runtime_error(""); // exit to main() and return -1
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  if (config_.debug_enabled) {
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  }

  /// Create GLFW window
  window_ =
      glfwCreateWindow(config_.window_width, config_.window_height, config_.window_name.c_str(), nullptr, nullptr);
  if (!window_) {
    std::cerr << "Initialization ERROR: Failed to create GLFW window object." << std::endl;
    glfwTerminate();
    throw std::runtime_error(""); // exit to main() and return -1
  }
  glfwMakeContextCurrent(window_);
  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetWindowPos(window_, config_.window_initial_x_pos, config_.window_initial_y_pos);

  /// Register GLFW callbacks
  // WindowUserPointer allows us to reference this class instance from static callback functions
  glfwSetWindowUserPointer(window_, reinterpret_cast<void*>(this));
  auto staticFramebufferSizeCallback = [](GLFWwindow* w, int width, int height) {
    reinterpret_cast<Initializer*>(glfwGetWindowUserPointer(w))->framebufferSizeCallback(width, height);
  };
  auto staticCursorPosCallback = [](GLFWwindow* w, double x, double y) {
    reinterpret_cast<Initializer*>(glfwGetWindowUserPointer(w))->cursorPosCallback(static_cast<float>(x),
                                                                                   static_cast<float>(y));
  };
  auto staticScrollCallback = [](GLFWwindow* w, double x, double y) {
    reinterpret_cast<Initializer*>(glfwGetWindowUserPointer(w))->scrollCallback(static_cast<float>(y));
  };
  glfwSetFramebufferSizeCallback(window_, staticFramebufferSizeCallback);
  glfwSetCursorPosCallback(window_, staticCursorPosCallback);
  glfwSetScrollCallback(window_, staticScrollCallback);

  /// Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    glfwTerminate();
    throw std::runtime_error("Failed to initialize GLAD.");
  }

  /// Configure OpenGL debug output
  int flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) { // Check if GLFW created a debug context (i.e. if debug_enabled = true)
    std::cout << "OpenGL Debug Output enabled" << std::endl;
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    if (config_.debug_level >= DebugLevel::LOW) {
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    }
    if (config_.debug_level >= DebugLevel::MEDIUM) {
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
    }
    if (config_.debug_level >= DebugLevel::HIGH) {
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_FALSE);
    }
  }
}

void Initializer::processKeyboardInput() {
  if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window_, true);
}

void Initializer::framebufferSizeCallback(int width, int height) {
  config_.window_width = width;
  config_.window_height = height;
  if (!lock_gl_viewport) glViewport(0, 0, width, height);
}

void APIENTRY Initializer::debugMessageCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                                GLsizei length, const char* message, const void* user_param) {
  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << ")";
  std::cout << " | Source: ";
  switch (source) {
    case GL_DEBUG_SOURCE_API:
      std::cout << "API";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      std::cout << "Window System";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      std::cout << "Shader Compiler";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      std::cout << "Third Party";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      std::cout << "Application";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      std::cout << "Other";
      break;
    default:
      std::cout << "Unrecognized GLenum value " << source;
      break;
  }
  std::cout << " | Type: ";
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      std::cout << "Error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      std::cout << "Deprecated Behaviour";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      std::cout << "Undefined Behaviour";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      std::cout << "Portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      std::cout << "Performance";
      break;
    case GL_DEBUG_TYPE_MARKER:
      std::cout << "Marker";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      std::cout << "Push Group";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      std::cout << "Pop Group";
      break;
    case GL_DEBUG_TYPE_OTHER:
      std::cout << "Other";
      break;
    default:
      std::cout << "Unrecognized GLenum value " << type;
      break;
  }
  std::cout << " | Severity: ";
  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      std::cout << "High";
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      std::cout << "Medium";
      break;
    case GL_DEBUG_SEVERITY_LOW:
      std::cout << "Low";
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      std::cout << "Notification";
      break;
    default:
      std::cout << "Unrecognized GLenum value " << severity;
      break;
  }
  std::cout << std::endl << "Message: " << message << std::endl;
}
