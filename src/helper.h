#ifndef TEMPLEGL_SRC_HELPER_H_
#define TEMPLEGL_SRC_HELPER_H_
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void APIENTRY debugMessageCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                   GLsizei length, const char *message, const void *userParam);

#endif //TEMPLEGL_SRC_HELPER_H_
