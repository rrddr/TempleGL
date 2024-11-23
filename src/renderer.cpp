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
    const YAML::Node camera_values = config_yaml["camera"]["initial_values"];
    const auto init_pos_vector = camera_values["position"].as<std::vector<float>>();
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

    const YAML::Node camera_limits = config_yaml["camera"]["limits"];
    config_.max_camera_speed = camera_limits["max_speed"].as<float>();

    const YAML::Node camera_frustum = config_yaml["camera"]["view_frustum"];
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
  glCreateVertexArrays(1, &objects_.vao.id);
  glBindVertexArray(objects_.vao.id);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  state_.first_time_receiving_mouse_input = true;
  state_.current_time = static_cast<float>(glfwGetTime());
  const auto aspect_ratio = static_cast<float>(config_.window_width) / static_cast<float>(config_.window_height);
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
  shadow_map_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages()
                                                           .vertex(config_.shader_path + "/shadow.vert")
                                                           .fragment(config_.shader_path + "/shadow.frag"));
  temple_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages()
                                                       .vertex(config_.shader_path + "/blinn_phong.vert")
                                                       .fragment(config_.shader_path + "/blinn_phong.frag"));
  skybox_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages()
                                                       .vertex(config_.shader_path + "/sky.vert")
                                                       .fragment(config_.shader_path + "/sky.frag"));
  image_shader_ = std::make_unique<ShaderProgram>(ShaderProgram::Stages()
                                                      .vertex(config_.shader_path + "/image_space.vert")
                                                      .fragment(config_.shader_path + "/image_space.frag"));

  glCreateFramebuffers(1, &objects_.scene_fbo.id);
  createSceneFramebufferAttachments();
  glCreateFramebuffers(1, &objects_.shadow_fbo.id);
  createShadowFramebufferAttachments();
  initializeMatrixBuffer();
  initializeLightBuffer();

  temple_model_->drawSetup(TEMPLE_VERTEX_SSBO_BINDING, TEMPLE_TEXTURE_ARRAY_BINDING);
  skybox_->drawSetup(SKYBOX_VERTEX_SSBO_BINDING, SKYBOX_CUBE_MAP_BINDING);

  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                       "Renderer::renderSetup() successful.");
}

void Renderer::updateRenderState() {
  const auto new_time = static_cast<float>(glfwGetTime());
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
  /// Compute shadows
  generateShadowMap();

  /// Render scene to framebuffer
  lock_gl_viewport_ = true;
  glBindFramebuffer(GL_FRAMEBUFFER, objects_.scene_fbo.id);
  glClear(GL_DEPTH_BUFFER_BIT);
  glNamedFramebufferDrawBuffer(objects_.scene_fbo.id, GL_COLOR_ATTACHMENT0);
  glClear(GL_COLOR_BUFFER_BIT);
  temple_model_->draw(temple_shader_);
  glNamedFramebufferDrawBuffer(objects_.scene_fbo.id, GL_COLOR_ATTACHMENT1);
  glClear(GL_COLOR_BUFFER_BIT);
  skybox_->draw(skybox_shader_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  lock_gl_viewport_ = false;
  processPendingGlViewport();

  /// Post-processing and render to screen
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

void Renderer::initializeMatrixBuffer() {
  glCreateBuffers(1, &state_.matrix_buffer.id);
  glNamedBufferStorage(state_.matrix_buffer.id,
                       3 * sizeof(glm::mat4),
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
}

void Renderer::initializeLightBuffer() {
  std::vector<Light> point_lights;
  for (glm::vec4 position : temple_model_->light_positions_) {
    point_lights.emplace_back(position, DEFAULT_POINT_LIGHT.color, DEFAULT_POINT_LIGHT.intensity);
  }

  glCreateBuffers(1, &state_.light_buffer.id);
  glNamedBufferStorage(state_.light_buffer.id,
                       static_cast<GLsizeiptr>(
                           sizeof(glm::vec4)
                           + sizeof(Light)
                           + sizeof(glm::vec4) // sizeof(GLuint) + padding to maintain std430 alignment
                           + point_lights.size() * sizeof(Light)),
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
  const auto num_point_lights = static_cast<GLuint>(point_lights.size());
  glNamedBufferSubData(state_.light_buffer.id,
                       sizeof(glm::vec4)
                       + sizeof(Light),
                       sizeof(GLuint),
                       &num_point_lights);
  glNamedBufferSubData(state_.light_buffer.id,
                       sizeof(glm::vec4)
                       + sizeof(Light)
                       + sizeof(glm::vec4),
                       static_cast<GLsizeiptr>(point_lights.size() * sizeof(Light)),
                       point_lights.data());
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHT_SSBO_BINDING, state_.light_buffer.id);
}

void Renderer::createSceneFramebufferAttachments() {
  // Attachments must be recreated if window dimensions change
  if (objects_.scene_fbo_color0.id) glDeleteTextures(1, &objects_.scene_fbo_color0.id);
  if (objects_.scene_fbo_depth.id) glDeleteRenderbuffers(1, &objects_.scene_fbo_depth.id);

  glCreateTextures(GL_TEXTURE_2D, 1, &objects_.scene_fbo_color0.id);
  glTextureParameteri(objects_.scene_fbo_color0.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(objects_.scene_fbo_color0.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureStorage2D(objects_.scene_fbo_color0.id,
                     1,
                     GL_RGBA16F,
                     config_.window_width,
                     config_.window_height);
  glNamedFramebufferTexture(objects_.scene_fbo.id, GL_COLOR_ATTACHMENT0, objects_.scene_fbo_color0.id, 0);

  glCreateTextures(GL_TEXTURE_2D, 1, &objects_.scene_fbo_color1.id);
  glTextureParameteri(objects_.scene_fbo_color1.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(objects_.scene_fbo_color1.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureStorage2D(objects_.scene_fbo_color1.id,
                     1,
                     GL_RGBA16F,
                     config_.window_width,
                     config_.window_height);
  glNamedFramebufferTexture(objects_.scene_fbo.id, GL_COLOR_ATTACHMENT1, objects_.scene_fbo_color1.id, 0);

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

  glBindTextureUnit(IMAGE_SCENE_TEXTURE_BINDING, objects_.scene_fbo_color0.id);
  glBindTextureUnit(SKY_SCENE_TEXTURE_BINDING, objects_.scene_fbo_color1.id);
}

void Renderer::createShadowFramebufferAttachments() {
  glCreateTextures(GL_TEXTURE_2D, 1, &objects_.shadow_fbo_depth.id);
  glTextureParameteri(objects_.shadow_fbo_depth.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(objects_.shadow_fbo_depth.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(objects_.shadow_fbo_depth.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTextureParameteri(objects_.shadow_fbo_depth.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  constexpr float borderColor[] {1.0f, 1.0f, 1.0f, 1.0f};
  glTextureParameterfv(objects_.shadow_fbo_depth.id, GL_TEXTURE_BORDER_COLOR, borderColor);
  glTextureStorage2D(objects_.shadow_fbo_depth.id,
                     1,
                     GL_DEPTH_COMPONENT32F,
                     SHADOW_MAP_SIZE,
                     SHADOW_MAP_SIZE);
  glNamedFramebufferTexture(objects_.shadow_fbo.id, GL_DEPTH_ATTACHMENT, objects_.shadow_fbo_depth.id, 0);
  checkFramebufferErrors(objects_.shadow_fbo);

  glBindTextureUnit(SUNLIGHT_SHADOW_MAP_BINDING, objects_.shadow_fbo_depth.id);
}

void Renderer::generateShadowMap() {
  /// Compute light view matrix
  std::vector<glm::vec4> corners = getFrustumCorners(camera_->getProjectionMatrix(), camera_->getViewMatrix());
  glm::vec3 frustum_center {0.0f};
  for (const glm::vec4& corner : corners) {
    frustum_center += glm::vec3(corner);
  }
  frustum_center /= corners.size();
  const glm::mat4 light_view = glm::lookAt(
      frustum_center + glm::vec3(SUNLIGHT.source),
      frustum_center,
      glm::vec3(0.0f, 1.0f, 0.0f));

  /// Compute light projection matrix
  float min_x = std::numeric_limits<float>::max();
  float min_y = std::numeric_limits<float>::max();
  float max_x = std::numeric_limits<float>::lowest();
  float max_y = std::numeric_limits<float>::lowest();
  for (const glm::vec4& corner : corners) {
    const glm::vec4 light_view_corner = light_view * corner;
    min_x = glm::min(min_x, light_view_corner.x);
    min_y = glm::min(min_y, light_view_corner.y);
    max_x = glm::max(max_x, light_view_corner.x);
    max_y = glm::max(max_y, light_view_corner.y);
  }
  const float z_depth = config_.camera_far_plane * 2.0;
  const glm::mat4 light_projection = glm::ortho(min_x, max_x, min_y, max_y, -z_depth, z_depth);
  const glm::mat4 light_space = light_projection * light_view;
  glNamedBufferSubData(state_.matrix_buffer.id,
                       2 * sizeof(glm::mat4),
                       sizeof(glm::mat4),
                       glm::value_ptr(light_space));

  /// Generate
  lock_gl_viewport_ = true;
  glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
  glBindFramebuffer(GL_FRAMEBUFFER, objects_.shadow_fbo.id);
  glNamedFramebufferDrawBuffer(objects_.shadow_fbo.id, GL_NONE);
  glClear(GL_DEPTH_BUFFER_BIT);
  temple_model_->draw(shadow_map_shader_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, config_.window_width, config_.window_height);
  lock_gl_viewport_ = false;
  processPendingGlViewport();
}

void Renderer::framebufferSizeCallback(int width, int height) {
  Initializer::framebufferSizeCallback(width, height);
  if (!lock_gl_viewport_) { createSceneFramebufferAttachments(); }
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

std::vector<glm::vec4> Renderer::getFrustumCorners(const glm::mat4& projection, const glm::mat4& view) {
  std::vector<glm::vec4> corners;
  const auto inverse = glm::inverse(projection * view);
  for (unsigned char b = 0x00; b < 0x08; ++b) {
    const glm::vec4 ndc_corner {
        static_cast<float>(b & 0x01) * 2.0f - 1.0f,
        static_cast<float>((b >> 1) & 0x01) * 2.0f - 1.0f,
        static_cast<float>((b >> 2) & 0x01) * 2.0f - 1.0f,
        1.0f
    };
    const glm::vec4 world_space_corner = inverse * ndc_corner;
    corners.push_back(world_space_corner / world_space_corner.w);
  }
  return corners;
}
void Renderer::processPendingGlViewport() {
  if (pending_gl_viewport_) {
    glViewport(0, 0, config_.window_width, config_.window_height);
    createSceneFramebufferAttachments();
    pending_gl_viewport_ = false;
  }
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