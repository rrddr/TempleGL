#include "renderer.h"

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

#include <iostream>
#include <vector>

void Renderer::loadConfigYaml() {
  Initializer::loadConfigYaml();
  // We accept having to load the file a second time, as this is cleaner than any alternative
  YAML::Node config_yaml;
  try {
    config_yaml = YAML::LoadFile("../config.yaml");
  }
  catch (YAML::Exception& e) {
    std::cerr << "ERROR (Renderer::loadConfigYaml): Failed to load config.yaml." << std::endl;
    throw; // re-throw to main
  }
  try {
    YAML::Node camera_values = config_yaml["camera"]["initial_values"];
    auto init_pos_vector = camera_values["position"].as<std::vector<float>>();
    if (init_pos_vector.size() != 3) {
      std::cerr << "WARNING (Renderer::loadConfigYaml): invalid setting in config.yaml, "
                << "camera.initial_values.position_ must be an array of exactly 3 floats."
                << "Defaulting to initial position_ [0.0, 0.0, 0.0]." << std::endl;
      config_.initial_camera_pos = glm::vec3(0.0f);
    } else {
      config_.initial_camera_pos = glm::vec3(init_pos_vector[0], init_pos_vector[1], init_pos_vector[2]);
    }
    config_.initial_camera_yaw = glm::radians(camera_values["yaw"].as<float>());
    config_.initial_camera_pitch = glm::radians(camera_values["pitch"].as<float>());
    config_.initial_camera_speed = camera_values["speed"].as<float>();

    YAML::Node camera_limits = config_yaml["camera"]["limits"];
    config_.max_camera_speed = camera_limits["max_speed"].as<float>();

    YAML::Node camera_frustum = config_yaml["camera"]["view_frustum"];
    config_.camera_fov = glm::radians(camera_frustum["fov"].as<float>());
    config_.camera_near_plane = camera_frustum["near_plane"].as<float>();
    config_.camera_far_plane = camera_frustum["far_plane"].as<float>();

    config_.model_path = config_yaml["model"]["source_path"].as<std::string>();
    config_.shader_path = config_yaml["shader"]["source_path"].as<std::string>();
  }
  catch (YAML::Exception& e) {
    std::cerr << "ERROR (Renderer::loadConfigYaml): Failed to parse config.yaml." << std::endl;
    throw; // re-throw to main
  }
}

void Renderer::renderSetup() {
  // Bind a non-zero VAO to avoid errors, see https://www.khronos.org/opengl/wiki/Vertex_Rendering/Rendering_Failure
  glCreateVertexArrays(1, &vao_.id);
  glBindVertexArray(vao_.id);

  state_.first_time_receiving_mouse_input = true;
  state_.current_time = static_cast<float>(glfwGetTime());
  auto aspect_ratio = static_cast<float>(config_.window_width) / static_cast<float>(config_.window_height);
  camera_ = std::make_unique<Camera>(config_.initial_camera_pos,
                                     config_.initial_camera_yaw,
                                     config_.initial_camera_pitch,
                                     config_.initial_camera_speed,
                                     config_.max_camera_speed,
                                     config_.camera_fov,
                                     aspect_ratio,
                                     config_.camera_near_plane,
                                     config_.camera_far_plane);
  temple_model_ = std::make_unique<Model>(config_.model_path + "/minecraft.obj");
  basic_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages()
                                                      .vertex(config_.shader_path + "/basic.vert")
                                                      .fragment(config_.shader_path + "/basic.frag"));
  glEnable(GL_DEPTH_TEST);

  temple_model_->drawSetup(basic_shader_);

  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                       "Renderer::renderSetup() successful.");
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

  basic_shader_->use();
  basic_shader_->setMat4("view", camera_->getViewMatrix());
  basic_shader_->setMat4("projection", camera_->getProjectionMatrix()); // may change on window resize
  temple_model_->draw();
}

void Renderer::renderTerminate() {
  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                       "Renderer::renderTerminate() successful.");
}

void Renderer::framebufferSizeCallback(int width, int height) {
  Initializer::framebufferSizeCallback(width, height);
  camera_->aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);
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
