#include "renderer.h"

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <format>

void Renderer::loadConfigYaml() {
  Initializer::loadConfigYaml();
  YAML::Node config_yaml;
  try {
    config_yaml = YAML::LoadFile("../config.yaml");
  } catch (YAML::Exception&) {
    std::cerr << "ERROR (Renderer::loadConfigYaml): Failed to load config.yaml." << std::endl;
    throw; // re-throw to main
  }
  try {
    const auto initial_pos_vector =
      config_yaml["camera"]["initial_values"]["position"].as<std::vector<float>>();
    if (initial_pos_vector.size() != 3) {
      std::cerr << "WARNING (Renderer::loadConfigYaml): invalid setting in config.yaml, "
                << "camera.initial_values.position must be an array of exactly 3 floats."
                << "Defaulting to initial position [0.0, 0.0, 0.0]." << std::endl;
      config_.initial_camera_pos = glm::vec3(0.0f);
    } else {
      config_.initial_camera_pos = glm::vec3(initial_pos_vector[0], initial_pos_vector[1], initial_pos_vector[2]);
    }
    config_.initial_camera_yaw           = glm::radians(config_yaml["camera"]["initial_values"]["yaw"].as<float>());
    config_.initial_camera_pitch         = glm::radians(config_yaml["camera"]["initial_values"]["pitch"].as<float>());
    config_.initial_camera_speed         = config_yaml["camera"]["initial_values"]["speed"].as<float>();
    config_.max_camera_speed             = config_yaml["camera"]["limits"]["max_speed"].as<float>();
    config_.camera_fov                   = glm::radians(config_yaml["camera"]["view_frustum"]["fov"].as<float>());
    config_.camera_near_plane            = config_yaml["camera"]["view_frustum"]["near_plane"].as<float>();
    config_.camera_far_plane             = config_yaml["camera"]["view_frustum"]["far_plane"].as<float>();
    config_.model_source_path            = config_yaml["model"]["source_path"].as<std::string>();
    config_.shader_source_path           = config_yaml["shader"]["source_path"].as<std::string>();
    config_.debug_render_light_positions = config_yaml["debug"]["render_light_positions"].as<bool>();
  } catch (YAML::Exception&) {
    std::cerr << "ERROR (Renderer::loadConfigYaml): Failed to parse config.yaml." << std::endl;
    throw; // re-throw to main
  }
}

void Renderer::renderSetup() {
  // Bind a non-zero VAO to avoid errors, see https://www.khronos.org/opengl/wiki/Vertex_Rendering/Rendering_Failure
  glCreateVertexArrays(1, &objects_.vao.id);
  glBindVertexArray(objects_.vao.id);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  state_.first_time_receiving_mouse_input = true;
  state_.current_time                     = static_cast<float>(glfwGetTime());
  const auto aspect_ratio {static_cast<float>(config_.window_width) / static_cast<float>(config_.window_height)};
  camera_ = std::make_unique<Camera>(config_.initial_camera_pos,
                                     config_.initial_camera_yaw,
                                     config_.initial_camera_pitch,
                                     config_.initial_camera_speed,
                                     config_.max_camera_speed,
                                     config_.camera_fov,
                                     aspect_ratio,
                                     config_.camera_near_plane,
                                     config_.camera_far_plane);
  temple_model_ = std::make_unique<Model>(config_.model_source_path + "temple/");
  const std::vector skybox_paths {
    config_.model_source_path + "skybox/px.png",
    config_.model_source_path + "skybox/nx.png",
    config_.model_source_path + "skybox/py.png",
    config_.model_source_path + "skybox/ny.png",
    config_.model_source_path + "skybox/pz.png",
    config_.model_source_path + "skybox/nz.png"
  };
  skybox_     = std::make_unique<Skybox>(skybox_paths);
  csm_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages(config_.shader_source_path, SHADER_CONSTANTS)
                                                .vertex("csm.vert")
                                                .geometry("csm.geom")
                                                .fragment("empty.frag"));
  temple_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages(config_.shader_source_path, SHADER_CONSTANTS)
                                                   .vertex("blinn_phong.vert")
                                                   .fragment("blinn_phong.frag"));
  skybox_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages(config_.shader_source_path, SHADER_CONSTANTS)
                                                   .vertex("sky.vert")
                                                   .fragment("sky.frag"));
  image_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages(config_.shader_source_path, SHADER_CONSTANTS)
                                                  .vertex("image_space.vert")
                                                  .fragment("image_space.frag"));
  if (config_.debug_enabled && config_.debug_render_light_positions) {
    debug_light_positions_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages(
                                                                     config_.shader_source_path,
                                                                     SHADER_CONSTANTS)
                                                                    .vertex("debug_lights.vert")
                                                                    .geometry("debug_lights.geom")
                                                                    .fragment("debug_lights.frag"));
  }

  initializeMatrixBuffer();
  initializeLightDataBuffer();
  glCreateFramebuffers(1, &objects_.scene_fbo.id);
  createSceneFramebufferAttachments();
  initializeCSMFramebuffer();

  temple_model_->drawSetup(SSBOBinding::TEMPLE_VERTEX, TextureBinding::TEMPLE_ARRAY);
  skybox_->drawSetup(SSBOBinding::SKY_VERTEX, TextureBinding::SKY_CUBE_MAP);

  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                       GL_DEBUG_TYPE_OTHER,
                       0,
                       GL_DEBUG_SEVERITY_NOTIFICATION,
                       -1,
                       "(Renderer::renderSetup): Completed successfully.");
}

void Renderer::updateRenderState() {
  const auto new_time {static_cast<float>(glfwGetTime())};
  state_.delta_time   = new_time - state_.current_time;
  state_.current_time = new_time;
  camera_->updateViewMatrix();
  glNamedBufferSubData(objects_.matrix_buffer.id,
                       sizeof(glm::mat4),
                       sizeof(glm::mat4),
                       glm::value_ptr(camera_->getViewMatrix()));
  glNamedBufferSubData(objects_.light_data_buffer.id,
                       0,
                       sizeof(glm::vec4),
                       glm::value_ptr(glm::vec4(camera_->getPosition(), 1.0f)));
}

void Renderer::processKeyboardInput() {
  Initializer::processKeyboardInput();
  if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) camera_->processKeyboard(Camera::FORWARD, state_.delta_time);
  if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) camera_->processKeyboard(Camera::BACKWARD, state_.delta_time);
  if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) camera_->processKeyboard(Camera::LEFT, state_.delta_time);
  if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) camera_->processKeyboard(Camera::RIGHT, state_.delta_time);
  if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS) camera_->processKeyboard(Camera::UP, state_.delta_time);
  if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    camera_->processKeyboard(Camera::DOWN, state_.delta_time);
}

void Renderer::render() {
  /// Compute sunlight shadows
  renderSunlightCSM();

  /// Render scene to framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, objects_.scene_fbo.id);
  glClear(GL_DEPTH_BUFFER_BIT);
  glNamedFramebufferDrawBuffer(objects_.scene_fbo.id, GL_COLOR_ATTACHMENT0 + SCENE_FBO_COLOR_INDEX_TEMPLE);
  glClear(GL_COLOR_BUFFER_BIT);
  temple_model_->draw(temple_shader_);
  glNamedFramebufferDrawBuffer(objects_.scene_fbo.id, GL_COLOR_ATTACHMENT0 + SCENE_FBO_COLOR_INDEX_SKY);
  glClear(GL_COLOR_BUFFER_BIT);
  skybox_->draw(skybox_shader_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  /// Post-processing and render to screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  image_shader_->use();
  glDisable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  if (config_.debug_enabled && config_.debug_render_light_positions) {
    debug_light_positions_shader_->use();
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(std::ssize(temple_model_->light_positions_)));
  }
  glEnable(GL_DEPTH_TEST);
}

void Renderer::renderTerminate() {
  // Placeholder. Thanks to RAII there is nothing that needs to be done here right now
  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                       GL_DEBUG_TYPE_OTHER,
                       0,
                       GL_DEBUG_SEVERITY_NOTIFICATION,
                       -1,
                       "(Renderer::renderTerminate): Completed successfully.");
}

void Renderer::initializeMatrixBuffer() {
  glCreateBuffers(1, &objects_.matrix_buffer.id);
  glNamedBufferStorage(objects_.matrix_buffer.id,
                       (2 + CSM_NUM_CASCADES) * sizeof(glm::mat4),
                       nullptr,
                       GL_DYNAMIC_STORAGE_BIT);
  glNamedBufferSubData(objects_.matrix_buffer.id,
                       0,
                       sizeof(glm::mat4),
                       glm::value_ptr(camera_->getProjectionMatrix()));
  glNamedBufferSubData(objects_.matrix_buffer.id,
                       sizeof(glm::mat4),
                       sizeof(glm::mat4),
                       glm::value_ptr(camera_->getViewMatrix()));
  glBindBufferBase(GL_UNIFORM_BUFFER, UBOBinding::MATRIX, objects_.matrix_buffer.id);
}

void Renderer::initializeLightDataBuffer() {
  std::vector<Light> point_lights;
  point_lights.reserve(temple_model_->light_positions_.size());
  for (const glm::vec4& position : temple_model_->light_positions_) {
    point_lights.emplace_back(position, DEFAULT_POINT_LIGHT.color, DEFAULT_POINT_LIGHT.intensity);
  }

  glCreateBuffers(1, &objects_.light_data_buffer.id);
  glNamedBufferStorage(objects_.light_data_buffer.id,
                       static_cast<GLsizeiptr>(sizeof(glm::vec4)
                                               + static_cast<size_t>(std::ceil(CSM_NUM_CASCADES / 4.0f))
                                               * sizeof(glm::vec4)
                                               + sizeof(Light)
                                               + sizeof(glm::vec4)
                                               + std::ssize(point_lights) * sizeof(Light)),
                       nullptr,
                       GL_DYNAMIC_STORAGE_BIT);
  glNamedBufferSubData(objects_.light_data_buffer.id,
                       0,
                       sizeof(glm::vec4),
                       glm::value_ptr(glm::vec4(camera_->getPosition(), 1.0f)));
  glNamedBufferSubData(objects_.light_data_buffer.id,
                       static_cast<GLintptr>(sizeof(glm::vec4)
                                             + static_cast<size_t>(std::ceil(CSM_NUM_CASCADES / 4.0f))
                                             * sizeof(glm::vec4)),
                       sizeof(Light),
                       &SUNLIGHT);
  const auto num_point_lights {static_cast<GLuint>(std::ssize(point_lights))};
  glNamedBufferSubData(objects_.light_data_buffer.id,
                       static_cast<GLintptr>(sizeof(glm::vec4)
                                             + static_cast<size_t>(std::ceil(CSM_NUM_CASCADES / 4.0f))
                                             * sizeof(glm::vec4)
                                             + sizeof(Light)),
                       sizeof(GLuint),
                       &num_point_lights);
  glNamedBufferSubData(objects_.light_data_buffer.id,
                       static_cast<GLintptr>(sizeof(glm::vec4)
                                             + static_cast<size_t>(std::ceil(CSM_NUM_CASCADES / 4.0f))
                                             * sizeof(glm::vec4)
                                             + sizeof(Light)
                                             + sizeof(glm::vec4)),
                       static_cast<GLsizeiptr>(std::ssize(point_lights) * sizeof(Light)),
                       point_lights.data());
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOBinding::LIGHT_DATA, objects_.light_data_buffer.id);
}

void Renderer::createSceneFramebufferAttachments() {
  if (objects_.scene_fbo_color.empty()) {
    objects_.scene_fbo_color.reserve(SCENE_FBO_NUM_COLOR_ATTACHMENTS);
    for (int i = 0; i < SCENE_FBO_NUM_COLOR_ATTACHMENTS; ++i) {
      objects_.scene_fbo_color.emplace_back(std::make_unique<wrap::Texture>(0));
    }
  }
  for (size_t i = 0; i < SCENE_FBO_NUM_COLOR_ATTACHMENTS; ++i) {
    glDeleteTextures(1, &objects_.scene_fbo_color[i]->id);
    glCreateTextures(GL_TEXTURE_2D, 1, &objects_.scene_fbo_color[i]->id);
    glTextureParameteri(objects_.scene_fbo_color[i]->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(objects_.scene_fbo_color[i]->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(objects_.scene_fbo_color[i]->id,
                       1,
                       GL_RGBA16F,
                       config_.window_width,
                       config_.window_height);
    glNamedFramebufferTexture(objects_.scene_fbo.id,
                              GL_COLOR_ATTACHMENT0 + i,
                              objects_.scene_fbo_color[i]->id,
                              0);
  }
  glDeleteRenderbuffers(1, &objects_.scene_fbo_depth.id);
  glCreateRenderbuffers(1, &objects_.scene_fbo_depth.id);
  glNamedRenderbufferStorage(objects_.scene_fbo_depth.id,
                             GL_DEPTH_COMPONENT32F,
                             config_.window_width,
                             config_.window_height);
  glNamedFramebufferRenderbuffer(objects_.scene_fbo.id,
                                 GL_DEPTH_ATTACHMENT,
                                 GL_RENDERBUFFER,
                                 objects_.scene_fbo_depth.id);

  checkFramebufferErrors(objects_.scene_fbo);

  glBindTextureUnit(TextureBinding::SCENE_TEMPLE, objects_.scene_fbo_color[SCENE_FBO_COLOR_INDEX_TEMPLE]->id);
  glBindTextureUnit(TextureBinding::SCENE_SKY, objects_.scene_fbo_color[SCENE_FBO_COLOR_INDEX_SKY]->id);
}

void Renderer::initializeCSMFramebuffer() {
  glCreateFramebuffers(1, &objects_.csm_fbo.id);
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &objects_.csm_fbo_depth.id);
  glTextureStorage3D(objects_.csm_fbo_depth.id,
                     1,
                     GL_DEPTH_COMPONENT32F,
                     CSM_TEX_SIZE,
                     CSM_TEX_SIZE,
                     CSM_NUM_CASCADES);
  glTextureParameteri(objects_.csm_fbo_depth.id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
  glTextureParameteri(objects_.csm_fbo_depth.id, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
  glTextureParameteri(objects_.csm_fbo_depth.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(objects_.csm_fbo_depth.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(objects_.csm_fbo_depth.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTextureParameteri(objects_.csm_fbo_depth.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  constexpr float border_color[] {1.0f, 1.0f, 1.0f, 1.0f};
  glTextureParameterfv(objects_.csm_fbo_depth.id, GL_TEXTURE_BORDER_COLOR, border_color);
  glNamedFramebufferTexture(objects_.csm_fbo.id, GL_DEPTH_ATTACHMENT, objects_.csm_fbo_depth.id, 0);
  checkFramebufferErrors(objects_.csm_fbo);

  glBindTextureUnit(TextureBinding::SUN_CSM_ARRAY, objects_.csm_fbo_depth.id);
}

void Renderer::renderSunlightCSM() const {
  /// Use Practical Split Scheme algorithm to determine view frustum split positions
  std::array<glm::mat4, CSM_NUM_CASCADES> light_matrices {};
  const float ratio {std::pow(config_.camera_far_plane / config_.camera_near_plane, 1.0f / CSM_NUM_CASCADES)};
  const float step {(config_.camera_far_plane - config_.camera_near_plane) / CSM_NUM_CASCADES};
  float split_log {config_.camera_near_plane};
  float split_uni {config_.camera_near_plane};
  float split_blend {config_.camera_near_plane};
  for (size_t i = 0; i < CSM_NUM_CASCADES; ++i) {
    const float split_prev {split_blend};
    split_log         *= ratio;
    split_uni         += step;
    split_blend       = (split_log + split_uni) / 2.0f;
    light_matrices[i] = getSunlightMatrixForCascade(split_prev, split_blend);
    glNamedBufferSubData(objects_.light_data_buffer.id,
                         static_cast<GLintptr>(sizeof(glm::vec4) + i * sizeof(GLfloat)),
                         sizeof(GLfloat),
                         &split_blend);
  }
  glNamedBufferSubData(objects_.matrix_buffer.id,
                       2 * sizeof(glm::mat4),
                       CSM_NUM_CASCADES * sizeof(glm::mat4),
                       light_matrices.data());

  /// Render shadow maps
  glViewport(0, 0, CSM_TEX_SIZE, CSM_TEX_SIZE);
  glBindFramebuffer(GL_FRAMEBUFFER, objects_.csm_fbo.id);
  glClear(GL_DEPTH_BUFFER_BIT);
  temple_model_->draw(csm_shader_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, config_.window_width, config_.window_height);
}

void Renderer::framebufferSizeCallback(const int width, const int height) {
  Initializer::framebufferSizeCallback(width, height);
  createSceneFramebufferAttachments();
  camera_->updateAspectRatio(static_cast<float>(width) / static_cast<float>(height));
  camera_->updateProjectionMatrix();
  glNamedBufferSubData(objects_.matrix_buffer.id,
                       0,
                       sizeof(glm::mat4),
                       glm::value_ptr(camera_->getProjectionMatrix()));
}

void Renderer::cursorPosCallback(const float x_pos, const float y_pos) {
  // This prevents a large camera jump on start
  if (state_.first_time_receiving_mouse_input) {
    state_.first_time_receiving_mouse_input = false;
  } else {
    camera_->processMouseMovement(x_pos - state_.mouse_x, state_.mouse_y - y_pos);
  }
  state_.mouse_x = x_pos;
  state_.mouse_y = y_pos;
}

void Renderer::scrollCallback(const float y_offset) {
  camera_->processMouseScroll(y_offset, state_.delta_time);
}

glm::mat4 Renderer::getSunlightMatrixForCascade(const float near_plane, const float far_plane) const {
  /// Compute partition projection matrix
  const glm::mat4 projection {glm::perspective(config_.camera_fov,
                                               static_cast<float>(config_.window_width)
                                               / static_cast<float>(config_.window_height),
                                               near_plane,
                                               far_plane)};
  /// Compute light view matrix
  const std::vector<glm::vec4> corners {getFrustumCorners(projection, camera_->getViewMatrix())};
  glm::vec3 frustum_center {0.0f};
  for (const glm::vec4& corner : corners) { frustum_center += glm::vec3(corner); }
  frustum_center /= std::ssize(corners);
  const glm::mat4 light_view {glm::lookAt(frustum_center + glm::vec3(SUNLIGHT.source),
                                          frustum_center,
                                          glm::vec3(0.0f, 1.0f, 0.0f))};

  /// Compute light projection matrix
  float min_x {std::numeric_limits<float>::max()};
  float min_y {std::numeric_limits<float>::max()};
  float max_x {std::numeric_limits<float>::lowest()};
  float max_y {std::numeric_limits<float>::lowest()};
  for (const glm::vec4& corner : corners) {
    const glm::vec4 light_view_corner {light_view * corner};
    min_x = glm::min(min_x, light_view_corner.x);
    min_y = glm::min(min_y, light_view_corner.y);
    max_x = glm::max(max_x, light_view_corner.x);
    max_y = glm::max(max_y, light_view_corner.y);
  }
  const float& z_depth {config_.camera_far_plane};
  const glm::mat4 light_projection {glm::ortho(min_x, max_x, min_y, max_y, -z_depth, z_depth)};
  return light_projection * light_view;
}

std::vector<glm::vec4> Renderer::getFrustumCorners(const glm::mat4& projection, const glm::mat4& view) {
  std::vector<glm::vec4> corners;
  const glm::mat4 inverse {glm::inverse(projection * view)};
  for (unsigned char b = 0x00; b < 0x08; ++b) {
    const glm::vec4 ndc_corner {b & 0x01 ? 1.0f : -1.0f,
                                b & 0x02 ? 1.0f : -1.0f,
                                b & 0x04 ? 1.0f : -1.0f,
                                1.0f};
    const glm::vec4 world_space_corner {inverse * ndc_corner};
    corners.push_back(world_space_corner / world_space_corner.w);
  }
  return corners;
}

void Renderer::checkFramebufferErrors(const wrap::Framebuffer& framebuffer) {
  switch (const GLenum status {glCheckNamedFramebufferStatus(framebuffer.id, GL_FRAMEBUFFER)}) {
    case GL_FRAMEBUFFER_COMPLETE:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_OTHER,
                           framebuffer.id,
                           GL_DEBUG_SEVERITY_NOTIFICATION,
                           -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer is complete.");
      break;
    case GL_FRAMEBUFFER_UNDEFINED:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           framebuffer.id,
                           GL_DEBUG_SEVERITY_HIGH,
                           -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer undefined.");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           framebuffer.id,
                           GL_DEBUG_SEVERITY_HIGH,
                           -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Incomplete Attachment. "
                           "All attachments must be attachment complete (empty attachments are complete by default).");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           framebuffer.id,
                           GL_DEBUG_SEVERITY_HIGH,
                           -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Missing Attachment. "
                           "At least one image must be attached.");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           framebuffer.id,
                           GL_DEBUG_SEVERITY_HIGH,
                           -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Multisample mismatch. "
                           "All attached images must have the same number of multisample samples, and use the same "
                           "fixed sample layout setting.");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           framebuffer.id,
                           GL_DEBUG_SEVERITY_HIGH,
                           -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Layered mismatch. "
                           "Either all or none of the attached images must be layered attachments.");
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           framebuffer.id,
                           GL_DEBUG_SEVERITY_HIGH,
                           -1,
                           "(Renderer::checkFramebufferErrors): Framebuffer Incomplete -> Unsupported combination of "
                           "attached image formats.");
      break;
    default:
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                           GL_DEBUG_TYPE_ERROR,
                           framebuffer.id,
                           GL_DEBUG_SEVERITY_HIGH,
                           -1,
                           std::format("(Renderer::checkFramebufferErrors): Unrecognized glCheckFramebufferStatus() "
                                       "return value {}", status).c_str());
      break;
  }
}
