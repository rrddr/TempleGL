#ifndef TEMPLEGL_SRC_HELPER_H_
#define TEMPLEGL_SRC_HELPER_H_
#include <glad/glad.h>
#include <GLFW/glfw3.h>

/**
 * This is a callback function for parsing OpenGL Debug Messages, and sending the information to stdout.
 * It should be registered using { glDebugMessageCallback() }.
 */
void APIENTRY debugMessageCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                   GLsizei length, const char *message, const void *user_param);

#endif //TEMPLEGL_SRC_HELPER_H_
