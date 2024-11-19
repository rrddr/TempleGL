#include "renderer.h"

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <format>

void Renderer::loadConfigYaml() {
  Initializer::loadConfigYaml();
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
                << "camera.initial_values.position must be an array of exactly 3 floats."
                << "Defaulting to initial position [0.0, 0.0, 0.0]." << std::endl;
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

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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
  temple_model_ = std::make_unique<Model>(config_.model_path + "/temple/minecraft.obj");
  std::vector<std::string> skybox_paths {
      config_.model_path + "/skybox/px.png",
      config_.model_path + "/skybox/nx.png",
      config_.model_path + "/skybox/py.png",
      config_.model_path + "/skybox/ny.png",
      config_.model_path + "/skybox/pz.png",
      config_.model_path + "/skybox/nz.png"
  };
  skybox_ = std::make_unique<Skybox>(skybox_paths);
  temple_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages()
                                                      .vertex(config_.shader_path + "/blinn_phong.vert")
                                                      .fragment(config_.shader_path + "/blinn_phong.frag"));
  skybox_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages()
                                                       .vertex(config_.shader_path + "/sky.vert")
                                                       .fragment(config_.shader_path + "/sky.frag"));
  image_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages()
                                                      .vertex(config_.shader_path + "/image_space.vert")
                                                      .fragment(config_.shader_path + "/image_space.frag"));

  glCreateFramebuffers(1, &fbo_.id);
  createFramebufferAttachments();
  initializeUniformBuffers();

  temple_model_->drawSetup(TEMPLE_VERTEX_SSBO_BINDING, TEMPLE_TEXTURE_ARRAY_BINDING);
  skybox_->drawSetup(SKYBOX_VERTEX_SSBO_BINDING, SKYBOX_CUBE_MAP_BINDING);

  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                       "Renderer::renderSetup() successful.");
}

void Renderer::updateRenderState() {
  auto new_time = static_cast<float>(glfwGetTime());
  state_.delta_time = new_time - state_.current_time;
  state_.current_time = new_time;
  glNamedBufferSubData(state_.matrix_buffer.id,
                       sizeof(glm::mat4),
                       sizeof(glm::mat4),
                       glm::value_ptr(camera_->getViewMatrix()));
  glNamedBufferSubData(state_.light_buffer.id,
                       0,
                       sizeof(glm::vec4),
                       glm::value_ptr(glm::vec4(camera_->getPosition(), 1.0f)));
}

void Renderer::processKeyboardInput() {
  Initializer::processKeyboardInput();
  if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
    camera_->processKeyboard(Camera::FORWARD, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
    camera_->processKeyboard(Camera::BACKWARD, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
    camera_->processKeyboard(Camera::LEFT, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
    camera_->processKeyboard(Camera::RIGHT, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS) {
    camera_->processKeyboard(Camera::UP, state_.delta_time);
  }
  if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    camera_->processKeyboard(Camera::DOWN, state_.delta_time);
  }
}

void Renderer::render() {
  /// Render scene to framebuffer
  lock_gl_viewport_ = true;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_.id);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  temple_model_->draw(temple_shader_);
  skybox_->draw(skybox_shader_);

  /// Post-processing and render to screen
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  lock_gl_viewport_ = false;
  if (pending_gl_viewport_) {
    glViewport(0, 0, config_.window_width, config_.window_height);
    createFramebufferAttachments();
    pending_gl_viewport_ = false;
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  image_shader_->use();
  glDisable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glEnable(GL_DEPTH_TEST);
}

void Renderer::renderTerminate() {
  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                       "Renderer::renderTerminate() successful.");
}

void Renderer::framebufferSizeCallback(int width, int height) {
  Initializer::framebufferSizeCallback(width, height);
  if (!lock_gl_viewport_) { createFramebufferAttachments(); }
  camera_->updateAspectRatio(static_cast<float>(width) / static_cast<float>(height));
  glNamedBufferSubData(state_.matrix_buffer.id,
                       0,
                       sizeof(glm::mat4),
                       glm::value_ptr(camera_->getProjectionMatrix()));
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

void Renderer::initializeUniformBuffers() {
  glCreateBuffers(1, &state_.matrix_buffer.id);
  glNamedBufferStorage(state_.matrix_buffer.id,
                       2 * sizeof(glm::mat4),
                       nullptr,
                       GL_DYNAMIC_STORAGE_BIT);
  glNamedBufferSubData(state_.matrix_buffer.id,
                       0,
                       sizeof(glm::mat4),
                       glm::value_ptr(camera_->getProjectionMatrix()));
  glNamedBufferSubData(state_.matrix_buffer.id,
                       sizeof(glm::mat4),
                       sizeof(glm::mat4),
                       glm::value_ptr(camera_->getViewMatrix()));
  glBindBufferBase(GL_UNIFORM_BUFFER, MATRIX_UBO_BINDING, state_.matrix_buffer.id);

  std::vector<Light> point_lights;
  for (glm::vec4 position : temple_model_->light_positions_) {
    point_lights.emplace_back(position, DEFAULT_POINT_LIGHT.color, DEFAULT_POINT_LIGHT.intensity);
  }
  auto num_point_lights = static_cast<GLuint>(point_lights.size());

  glCreateBuffers(1, &state_.light_buffer.id);
  glNamedBufferStorage(state_.light_buffer.id,
                       sizeof(glm::vec4)
                       + sizeof(Light)
                       + sizeof(glm::vec4) // sizeof(GLuint) + padding to maintain std430 alignment
                       + point_lights.size() * sizeof(Light),
                       nullptr,
                       GL_DYNAMIC_STORAGE_BIT);
  glNamedBufferSubData(state_.light_buffer.id,
                       0,
                       sizeof(glm::vec4),
                       glm::value_ptr(glm::vec4(camera_->getPosition(), 1.0f)));
  glNamedBufferSubData(state_.light_buffer.id,
                       sizeof(glm::vec4),
                       sizeof(Light),
                       &SUNLIGHT);
  glNamedBufferSubData(state_.light_buffer.id,
                       sizeof(glm::vec4)
                       + sizeof(Light),
                       sizeof(GLuint),
                       &num_point_lights);
  glNamedBufferSubData(state_.light_buffer.id,
                       sizeof(glm::vec4)
                       + sizeof(Light)
                       + sizeof(glm::vec4),
                       point_lights.size() * sizeof(Light),
                       point_lights.data());
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHT_SSBO_BINDING, state_.light_buffer.id);
}

void Renderer::createFramebufferAttachments() {
  // Attachments must be recreated if window dimensions change
  if (fbo_color_attachment_.id) glDeleteTextures(1, &fbo_color_attachment_.id);
  if (fbo_depth_attachment_.id) glDeleteRenderbuffers(1, &fbo_depth_attachment_.id);

  glCreateTextures(GL_TEXTURE_2D, 1, &fbo_color_attachment_.id);
  glTextureStorage2D(fbo_color_attachment_.id,
                     1,
                     GL_RGBA16F,
                     config_.window_width,
                     config_.window_height);
  glTextureParameteri(fbo_color_attachment_.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(fbo_color_attachment_.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glCreateRenderbuffers(1, &fbo_depth_attachment_.id);
  glNamedRenderbufferStorage(fbo_depth_attachment_.id,
                             GL_DEPTH_COMPONENT32F,
                             config_.window_width,
                             config_.window_height);

  glNamedFramebufferTexture(fbo_.id, GL_COLOR_ATTACHMENT0, fbo_color_attachment_.id, 0);
  glNamedFramebufferRenderbuffer(fbo_.id, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo_depth_attachment_.id);
  checkFramebufferErrors(fbo_);

  glBindTextureUnit(IMAGE_SCENE_TEXTURE_BINDING, fbo_color_attachment_.id);
}

void Renderer::checkFramebufferErrors(const wrap::Framebuffer& framebuffer) {
  GLenum status = glCheckNamedFramebufferStatus(framebuffer.id, GL_FRAMEBUFFER);
  switch (status) {
    case GL_FRAMEBUFFER_COMPLETE:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, framebuffer.id,
                           GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer is complete.");
      break;
    case GL_FRAMEBUFFER_UNDEFINED:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, framebuffer.id, GL_DEBUG_SEVERITY_HIGH, -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer undefined.");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, framebuffer.id, GL_DEBUG_SEVERITY_HIGH, -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Incomplete Attachment. "
                           "All attachments must be attachment complete (empty attachments are complete by default).");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, framebuffer.id, GL_DEBUG_SEVERITY_HIGH, -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Missing Attachment. "
                           "At least one image must be attached.");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, framebuffer.id, GL_DEBUG_SEVERITY_HIGH, -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Multisample mismatch. "
                           "All attached images must have the same number of multisample samples, and use the same "
                           "fixed sample layout setting.");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, framebuffer.id, GL_DEBUG_SEVERITY_HIGH, -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Layered mismatch. "
                           "Either all or none of the attached images must be layered attachments.");
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, framebuffer.id, GL_DEBUG_SEVERITY_HIGH, -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Unsupported combination of "
                           "attached image formats.");
      break;
    default:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, framebuffer.id, GL_DEBUG_SEVERITY_HIGH, -1,
                           std::format("(Renderer::checkFramebufferErrors): Unrecognized glCheckFramebufferStatus() "
                                       "return value {}", status).c_str());
      break;
  }
}