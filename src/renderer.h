#ifndef TEMPLEGL_SRC_RENDERER_H_
#define TEMPLEGL_SRC_RENDERER_H_

#include "initializer.h"
#include "initializer.cpp" // Implementations of template methods must be visible at compile time
#include "camera.h"
#include "model.h"
#include "shader_program.h"

#include <glm/glm.hpp>

#include <string>
#include <memory>

namespace {
  struct RendererConfig : MinimalConfig {
    glm::vec3 initial_camera_pos;
    float initial_camera_yaw;
    float initial_camera_pitch;
    float initial_camera_speed;
    float max_camera_speed;
    std::string model_path;
    std::string shader_path;
  };
}

/**
 * Implements the non-boilerplate methods declared by the abstract Initializer class.
 */
class Renderer : public Initializer<RendererConfig> {
 private:
  struct State {
    bool first_time_receiving_mouse_input;
    float mouse_x;
    float mouse_y;
    float current_time;
    float delta_time;
  };
  State state_{};
  std::unique_ptr<Camera> camera_;
  std::unique_ptr<Model> temple_model_;
  std::unique_ptr<ShaderProgram> basic_shader_;

  /// Program stages
  void loadConfigYaml() override;
  void renderSetup() override;
  void updateRenderState() override;
  void processKeyboardInput() override;
  void render() override;
  void renderTerminate() override;

  /// Callbacks
  void cursorPosCallback(float x_pos, float y_pos) override;
  void scrollCallback(float y_offset) override;

  /// Helper methods
  /**
   * Computes transform matrix from view space to clip space.
   *
   * @param fov         Vertical field of view in radians (angle between top and bottom planes of view frustrum).
   * @param near_plane  Distance of near plane from camera.
   * @param far_plane   Distance of far plane from camera.
   */
  inline glm::mat4 getProjectionMatrix(float fov = 1.5f, float near_plane = 0.1f, float far_plane = 100.0f) const {
    return glm::perspective(fov, (float) config_.window_width / (float) config_.window_height, near_plane, far_plane);
  }
};

#endif //TEMPLEGL_SRC_RENDERER_H_
