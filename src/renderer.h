#ifndef TEMPLEGL_SRC_RENDERER_H_
#define TEMPLEGL_SRC_RENDERER_H_

#include "initializer.h"
#include "initializer.cpp" // Implementations of template methods must be visible at compile time
#include "camera.h"
#include "model.h"
#include "skybox.h"
#include "shader_program.h"
#include "opengl_wrappers.h"

#include <glm/glm.hpp>

#include <string>
#include <memory>

struct RendererConfig : MinimalInitializerConfig {
  glm::vec3 initial_camera_pos;
  float initial_camera_yaw;
  float initial_camera_pitch;
  float initial_camera_speed;
  float max_camera_speed;
  float camera_fov;
  float camera_near_plane;
  float camera_far_plane;
  std::string model_path;
  std::string shader_path;
};

/**
 * Implements the non-boilerplate methods declared by the abstract Initializer class.
 */
class Renderer : public Initializer<RendererConfig> {
  struct State {
    bool first_time_receiving_mouse_input;
    float mouse_x;
    float mouse_y;
    float current_time;
    float delta_time;
    wrap::Buffer matrix_buffer;
    wrap::Buffer light_buffer;
  };
  State state_ {};
  std::unique_ptr<Camera> camera_;
  std::unique_ptr<Model> temple_model_;
  std::unique_ptr<Skybox> skybox_;
  std::unique_ptr<ShaderProgram> basic_shader_;
  std::unique_ptr<ShaderProgram> skybox_shader_;
  wrap::VertexArray vao_ {};

  struct DirectionalLight {
    glm::vec4 origin;
    glm::vec4 color;
  };
  static constexpr DirectionalLight sunlight {
    {-0.4f, 0.8f, -1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
  };

  /// Program stages
  void loadConfigYaml() override;
  void renderSetup() override;
  void updateRenderState() override;
  void processKeyboardInput() override;
  void render() override;
  void renderTerminate() override;

  /// Callbacks
  void framebufferSizeCallback(int width, int height) override;
  void cursorPosCallback(float x_pos, float y_pos) override;
  void scrollCallback(float y_offset) override;
};
#endif //TEMPLEGL_SRC_RENDERER_H_
