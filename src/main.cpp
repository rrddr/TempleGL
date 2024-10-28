#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "helper.h"
#include "camera.h"
#include "screen.h"
#include "shader_program.h"
#include "model.h"

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
    screen = std::make_unique<Screen>(1920, 1080, "TempleGL", camera, debug_mode);
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
//    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE); // Except low sev.
  }

  /// Compile shaders
  ShaderProgram basic_shader(ShaderProgram::Stages()
                                 .vertex("../shaders/basic.vert")
                                 .fragment("../shaders/basic.frag"));

  /// Load model
  Model temple_model("../model/minecraft.obj");

  /// Prepare texture array
  GLuint texture_array;
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture_array);
  int texture_count = (int) temple_model.texture_data.size();
  glTextureStorage3D(texture_array, 1, GL_RGB8, 128, 128, texture_count);
  for (int index = 0 ; index < texture_count ; ++index) {
    glTextureSubImage3D(texture_array, 0, 0, 0, index, 128, 128, 1, GL_RGB, GL_UNSIGNED_BYTE,
                        (const void*) temple_model.texture_data[index].get());
  }
  glTextureParameteri(texture_array, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(texture_array, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  /// Prepare vertex data SSBO
  // Need a dummy VAO to avoid errors, see https://www.khronos.org/opengl/wiki/Vertex_Rendering/Rendering_Failure
  GLuint dummy_vao;
  glGenVertexArrays(1, &dummy_vao);
  glBindVertexArray(dummy_vao);

  GLuint vertex_data_buffer;
  glCreateBuffers(1, &vertex_data_buffer);
  glNamedBufferStorage(vertex_data_buffer,
                       sizeof(Model::Vertex) * temple_model.vertices.size(),
                       (const void*) temple_model.vertices.data(),
                       GL_DYNAMIC_STORAGE_BIT);

  /// Prepare EBO
  GLuint indices_buffer;
  glCreateBuffers(1, &indices_buffer);
  glNamedBufferStorage(indices_buffer,
                       sizeof (unsigned int) * temple_model.indices.size(),
                       (const void*) temple_model.indices.data(),
                       GL_DYNAMIC_STORAGE_BIT);

  /// Prepare draw command SSBO
  GLuint draw_command_buffer;
  glCreateBuffers(1, &draw_command_buffer);
  glNamedBufferStorage(draw_command_buffer,
                       sizeof(DrawElementsIndirectCommand) * temple_model.draw_commands.size(),
                       (const void*) temple_model.draw_commands.data(),
                       GL_DYNAMIC_STORAGE_BIT);

  /// Configure shaders and buffers for rendering
  basic_shader.use();
  basic_shader.setMat4("projection", screen->getProjectionMatrix());
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_data_buffer);
  glBindTextureUnit(0, texture_array);
  basic_shader.setInt("texture_array", 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_command_buffer);

  /// Main render loop
  while(!glfwWindowShouldClose(screen->glfw_window)) {
    screen->processKeyboardInput();
    basic_shader.setMat4("view", screen->camera->getViewMatrix());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        (const void*) nullptr,
        temple_model.draw_commands.size(),
        0);

    glfwSwapBuffers(screen->glfw_window);
    glfwPollEvents();
  }
  glDeleteBuffers(1, &vertex_data_buffer);
  glDeleteBuffers(1, &indices_buffer);
  glDeleteBuffers(1, &draw_command_buffer);
  glDeleteVertexArrays(1, &dummy_vao);
  glDeleteTextures(1, &texture_array);
  glfwTerminate();
  return 0;
}
