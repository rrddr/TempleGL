#include "initializer.h"

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

#include <iostream>

template <typename T>
void Initializer<T>::run() {
  loadConfigYaml();
  init();
  renderSetup();
  while (!glfwWindowShouldClose(window_)) {
    processKeyboardInput();
    updateRenderState();
    render();
    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
  renderTerminate();
  glfwTerminate();
}

template <typename T>
void Initializer<T>::loadConfigYaml() {
  YAML::Node config_yaml;
  try {
    config_yaml = YAML::LoadFile("../config.yaml");
  } catch (YAML::Exception&) {
    std::cerr << "ERROR (Initializer::loadConfigYaml): Failed to load config.yaml." << std::endl;
    throw; // re-throw to main
  }
  try {
    config_.window_name          = config_yaml["window"]["name"].as<std::string>();
    config_.window_width         = config_yaml["window"]["width"].as<int>();
    config_.window_height        = config_yaml["window"]["height"].as<int>();
    config_.window_initial_x_pos = config_yaml["window"]["initial_x_pos"].as<int>();
    config_.window_initial_y_pos = config_yaml["window"]["initial_y_pos"].as<int>();

    config_.debug_enabled = config_yaml["debug"]["enabled"].as<bool>();
    if (const auto debug_level_str {config_yaml["debug"]["level"].as<std::string>()};
        debug_level_str == "all") {
      config_.debug_level = ALL;
    } else if (debug_level_str == "low") {
      config_.debug_level = LOW;
    } else if (debug_level_str == "medium") {
      config_.debug_level = MEDIUM;
    } else if (debug_level_str == "high") {
      config_.debug_level = HIGH;
    } else {
      std::cerr << "WARNING (Initializer::loadConfigYaml): invalid setting in config.yaml, "
                << "debug.level must be one of 'all', 'low', 'medium', 'high'. Defaulting to 'all'." << std::endl;
      config_.debug_level = ALL;
    }
  } catch (YAML::Exception&) {
    std::cerr << "ERROR (Initializer::loadConfigYaml): Failed to parse config.yaml." << std::endl;
    throw; // re-throw to main
  }
}

template <typename T>
void Initializer<T>::init() {
  /// Initialize GLFW
  glfwSetErrorCallback(glfwErrorCallback);
  if (!glfwInit()) { throw std::runtime_error("ERROR (Initializer::init): failed to initialize GLFW."); }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  if (config_.debug_enabled) { glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); }

  /// Create GLFW window
  window_ = glfwCreateWindow(config_.window_width,
                             config_.window_height,
                             config_.window_name.c_str(),
                             nullptr,
                             nullptr);
  if (!window_) { throw std::runtime_error("ERROR (Initializer::init): Failed to create GLFW window object."); }
  glfwMakeContextCurrent(window_);
  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetWindowPos(window_, config_.window_initial_x_pos, config_.window_initial_y_pos);

  /// Register GLFW callbacks
  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* w, int width, int height) {
    static_cast<Initializer*>(glfwGetWindowUserPointer(w))->framebufferSizeCallback(width, height);
  });
  glfwSetCursorPosCallback(window_, [](GLFWwindow* w, double x, double y) {
    static_cast<Initializer*>(glfwGetWindowUserPointer(w))->cursorPosCallback(static_cast<float>(x),
                                                                              static_cast<float>(y));
  });
  glfwSetScrollCallback(window_, [](GLFWwindow* w, double x, double y) {
    static_cast<Initializer*>(glfwGetWindowUserPointer(w))->scrollCallback(static_cast<float>(y));
  });

  /// Initialize GLAD
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    throw std::runtime_error("ERROR (Initializer::init): Failed to initialize GLAD.");
  }

  /// Configure OpenGL debug output
  int flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) { // Check if GLFW created debug context (expected if debug_enabled == true)
    std::cout << "INFO (Initializer::init): OpenGL debug output enabled." << std::endl;
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    if (config_.debug_level >= LOW) {
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    }
    if (config_.debug_level >= MEDIUM) {
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
    }
    if (config_.debug_level >= HIGH) {
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_FALSE);
    }
  }

  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                       GL_DEBUG_TYPE_OTHER,
                       0,
                       GL_DEBUG_SEVERITY_NOTIFICATION,
                       -1,
                       "(Initializer::init): Completed successfully.");
}

template <typename T>
void Initializer<T>::processKeyboardInput() {
  if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window_, true);
}

template <typename T>
void Initializer<T>::framebufferSizeCallback(int width, int height) {
  config_.window_width  = width;
  config_.window_height = height;
  glViewport(0, 0, width, height);
}

template <typename T>
void Initializer<T>::glfwErrorCallback(const int error_code, const char* description) {
  std::cerr << "GLFW ERROR (" << error_code << "): " << description << std::endl;
}

template <typename T>
void APIENTRY Initializer<T>::debugMessageCallback(GLenum source,
                                                   GLenum type,
                                                   unsigned int id,
                                                   GLenum severity,
                                                   GLsizei length,
                                                   const char* message,
                                                   const void* user_param) {
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
      std::cout << "Unrecognized GLenum value" << source;
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
      std::cout << "Unrecognized GLenum value" << type;
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
      std::cout << "Unrecognized GLenum value" << severity;
      break;
  }
  std::cout << std::endl << "Message: " << message << std::endl << std::endl;
}
