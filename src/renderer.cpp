#include "renderer.h"

#include <glm/glm.hpp>

void Renderer::renderSetup() {
  /// Set necessary initial state
  state_.first_time_receiving_mouse_input = true;
  state_.current_time = static_cast<float>(glfwGetTime());

  /// OpenGL configuration
  glEnable(GL_DEPTH_TEST);

  /// Create camera object, load model, compile shaders
  camera_ = std::make_unique<Camera>(Camera(glm::vec3(0.0f, 0.1f, 0.0f)));
  temple_model_ = std::make_unique<Model>("../model/minecraft.obj");
  basic_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages()
                                                      .vertex("../shaders/basic.vert")
                                                      .fragment("../shaders/basic.frag"));

  /// Prepare texture array
  GLuint texture_array;
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture_array);
  int texture_count = (int) temple_model_->texture_data.size();
  glTextureStorage3D(texture_array, 1, GL_RGB8, 128, 128, texture_count);
  for (int index = 0 ; index < texture_count ; ++index) {
    glTextureSubImage3D(texture_array, 0, 0, 0, index, 128, 128, 1, GL_RGB, GL_UNSIGNED_BYTE,
                        (const void*) temple_model_->texture_data[index].get());
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
                       sizeof(Model::Vertex) * temple_model_->vertices.size(),
                       (const void*) temple_model_->vertices.data(),
                       GL_DYNAMIC_STORAGE_BIT);

  /// Prepare EBO
  GLuint indices_buffer;
  glCreateBuffers(1, &indices_buffer);
  glNamedBufferStorage(indices_buffer,
                       sizeof (unsigned int) * temple_model_->indices.size(),
                       (const void*) temple_model_->indices.data(),
                       GL_DYNAMIC_STORAGE_BIT);

  /// Prepare draw command SSBO
  GLuint draw_command_buffer;
  glCreateBuffers(1, &draw_command_buffer);
  glNamedBufferStorage(draw_command_buffer,
                       sizeof(DrawElementsIndirectCommand) * temple_model_->draw_commands.size(),
                       (const void*) temple_model_->draw_commands.data(),
                       GL_DYNAMIC_STORAGE_BIT);

  /// Configure shaders and buffers for rendering
  basic_shader_->use();
  basic_shader_->setMat4("projection", getProjectionMatrix());
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_data_buffer);
  glBindTextureUnit(0, texture_array);
  basic_shader_->setInt("texture_array", 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_command_buffer);
}

void Renderer::updateRenderState() {
  auto new_time = static_cast<float>(glfwGetTime());
  state_.delta_time = new_time - state_.current_time;
  state_.current_time = new_time;
}

void Renderer::processKeyboardInput() {
  Initializer::processKeyboardInput();
  if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
    camera_->processKeyboard(FORWARD, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
    camera_->processKeyboard(BACKWARD, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
    camera_->processKeyboard(LEFT, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
    camera_->processKeyboard(RIGHT, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS) {
    camera_->processKeyboard(UP, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    camera_->processKeyboard(DOWN, state_.delta_time);
  }
}

void Renderer::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  basic_shader_->setMat4("view", camera_->getViewMatrix());
  glMultiDrawElementsIndirect(
      GL_TRIANGLES,
      GL_UNSIGNED_INT,
      (const void*) nullptr,
      temple_model_->draw_commands.size(),
      0);
}
void Renderer::renderTerminate() {
}
void Renderer::cursorPosCallback(float x_pos, float y_pos) {
  // This prevents a large camera jump on start
  if (state_.first_time_receiving_mouse_input) {
    state_.first_time_receiving_mouse_input = false;
  } else {
    camera_->processMouseMovement(x_pos - state_.mouse_x, state_.mouse_y - y_pos);
  }
  state_.mouse_x = x_pos;
  state_.mouse_y = y_pos;
}

void Renderer::scrollCallback(float y_offset) {
  camera_->processMouseScroll(y_offset, state_.delta_time);
}
