#include "helper.h"
#include <iostream>

/**
 * This is a callback function for parsing OpenGL Debug Messages, and sending the information to stdout.
 * It should be registered using {glDebugMessageCallback}.
 */
void APIENTRY debugMessageCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                   GLsizei length, const char* message, const void* user_param) {
  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " << message << std::endl;

  std::cout << "Source: ";
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
  std::cout << std::endl << std::endl;
}