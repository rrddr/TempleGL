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
  struct OpenGLObjects {
    wrap::VertexArray vao {};

    wrap::Framebuffer scene_fbo {};
    wrap::Texture scene_fbo_color0 {};
    wrap::Texture scene_fbo_color1 {};
    wrap::Renderbuffer scene_fbo_depth {};

    wrap::Framebuffer shadow_fbo {};
    wrap::Texture shadow_fbo_depth {};
  };
  State state_ {};
  OpenGLObjects objects_ {};
  std::unique_ptr<Camera> camera_;
  std::unique_ptr<Model> temple_model_;
  std::unique_ptr<Skybox> skybox_;
  std::unique_ptr<ShaderProgram> shadow_map_shader_;
  std::unique_ptr<ShaderProgram> temple_shader_;
  std::unique_ptr<ShaderProgram> skybox_shader_;
  std::unique_ptr<ShaderProgram> image_shader_;

  /// Main program stages
  void loadConfigYaml() override;
  void renderSetup() override;
  void updateRenderState() override;
  void processKeyboardInput() override;
  void render() override;
  void renderTerminate() override;

  /// Sub-stages
  void initializeMatrixBuffer();
  void initializeLightBuffer();
  void createSceneFramebufferAttachments();
  void createShadowFramebufferAttachments();
  void generateShadowMap();

  /// Callbacks
  void framebufferSizeCallback(int width, int height) override;
  void cursorPosCallback(float x_pos, float y_pos) override;
  void scrollCallback(float y_offset) override;

  /// Helper methods
  static std::vector<glm::vec4> getFrustumCorners(const glm::mat4& projection, const glm::mat4& view);
  static void checkFramebufferErrors(const wrap::Framebuffer& framebuffer);

  /// Hardcoded shader parameters
  struct Light {
    glm::vec4 source;
    glm::vec4 color;
    float intensity;
    float padding[3];   // padding to conform with std430 storage layout rules
  };
  static constexpr Light SUNLIGHT {
      {-0.4f, 0.9f, -1.0f, 0.0f},
      {1.0f, 0.7f, 0.4f, 1.0f},
      3.0f
  };
  static constexpr Light DEFAULT_POINT_LIGHT {
      {0.0f, 0.0f, 0.0f, 1.0f},
      {0.6f, 1.0f, 0.9f, 1.0f},
      0.0125f
  };
  static constexpr GLsizei SHADOW_MAP_SIZE {16192};

  static constexpr GLuint TEMPLE_TEXTURE_ARRAY_BINDING {0};
  static constexpr GLuint SKYBOX_CUBE_MAP_BINDING {1};
  static constexpr GLuint IMAGE_SCENE_TEXTURE_BINDING {2};
  static constexpr GLuint SKY_SCENE_TEXTURE_BINDING {3};
  static constexpr GLuint SUNLIGHT_SHADOW_MAP_BINDING {4};

  static constexpr GLuint TEMPLE_VERTEX_SSBO_BINDING {0};
  static constexpr GLuint SKYBOX_VERTEX_SSBO_BINDING {1};
  static constexpr GLuint LIGHT_SSBO_BINDING {2};

  static constexpr GLuint MATRIX_UBO_BINDING {0};
};
#endif //TEMPLEGL_SRC_RENDERER_H_
