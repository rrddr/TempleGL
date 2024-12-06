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
  std::string model_source_path;
  std::string shader_source_path;
  bool debug_render_light_positions;
};

/**
 * Implements the non-boilerplate methods declared by the abstract Initializer class.
 */
class Renderer final : public Initializer<RendererConfig> {
  struct State {
    bool first_time_receiving_mouse_input;
    float mouse_x;
    float mouse_y;
    float current_time;
    float delta_time;
  };
  struct OpenGLObjects {
    wrap::VertexArray vao;

    wrap::Framebuffer scene_fbo;
    std::vector<std::unique_ptr<wrap::Texture>> scene_fbo_color;
    wrap::Renderbuffer scene_fbo_depth;

    wrap::Framebuffer csm_fbo;
    wrap::Texture csm_fbo_depth;

    wrap::Buffer matrix_buffer;
    wrap::Buffer light_data_buffer;
  };
  State state_ {};
  OpenGLObjects objects_ {};
  std::unique_ptr<Camera> camera_;
  std::unique_ptr<Model> temple_model_;
  std::unique_ptr<Skybox> skybox_;
  std::unique_ptr<ShaderProgram> csm_shader_;
  std::unique_ptr<ShaderProgram> temple_shader_;
  std::unique_ptr<ShaderProgram> skybox_shader_;
  std::unique_ptr<ShaderProgram> image_shader_;
  std::unique_ptr<ShaderProgram> debug_light_positions_shader_;

  /// Main program stages
  void loadConfigYaml() override;
  void renderSetup() override;
  void updateRenderState() override;
  void processKeyboardInput() override;
  void render() override;
  void renderTerminate() override;

  /// Sub-stages
  void initializeMatrixBuffer();
  void initializeLightDataBuffer();
  void createSceneFramebufferAttachments(); // may be called multiple times
  void initializeCSMFramebuffer();
  void renderSunlightCSM() const;

  /// Callbacks
  void framebufferSizeCallback(int width, int height) override;
  void cursorPosCallback(float x_pos, float y_pos) override;
  void scrollCallback(float y_offset) override;

  /// Helper methods
  [[nodiscard]] glm::mat4 getSunlightMatrixForCascade(float near_plane, float far_plane) const;
  [[nodiscard]] static std::vector<glm::vec4> getFrustumCorners(const glm::mat4& projection, const glm::mat4& view);
  static void checkFramebufferErrors(const wrap::Framebuffer& framebuffer);

  /// Hardcoded shader parameters
  struct Light {
    glm::vec4 source;
    glm::vec4 color;
    float intensity;
    float padding[3]; // padding to conform with std430 storage layout rules
  };
  static constexpr Light SUNLIGHT {{-0.4f, 0.9f, -1.0f, 0.0f},
                                   {1.0f, 0.7f, 0.4f, 1.0f},
                                   3.0f};
  static constexpr Light DEFAULT_POINT_LIGHT {{0.0f, 0.0f, 0.0f, 1.0f},
                                              {0.6f, 1.0f, 0.9f, 1.0f},
                                              0.05f};
  static constexpr GLsizei CSM_TEX_SIZE {16192};
  static constexpr size_t CSM_NUM_CASCADES {3};

  static constexpr size_t SCENE_FBO_NUM_COLOR_ATTACHMENTS {2};
  static constexpr size_t SCENE_FBO_COLOR_INDEX_TEMPLE {0};
  static constexpr size_t SCENE_FBO_COLOR_INDEX_SKY {1};

  enum TextureBinding { TEMPLE_ARRAY, SUN_CSM_ARRAY, SKY_CUBE_MAP, SCENE_TEMPLE, SCENE_SKY };
  enum SSBOBinding { TEMPLE_VERTEX, SKY_VERTEX, LIGHT_DATA };
  enum UBOBinding { MATRIX };
  inline static const std::vector<std::pair<std::string, int>> SHADER_CONSTANTS {{
    std::make_pair("SAMPLER_ARRAY_TEMPLE", TEMPLE_ARRAY),
    std::make_pair("SAMPLER_ARRAY_SHADOW_SUN", SUN_CSM_ARRAY),
    std::make_pair("SAMPLER_CUBE_SKY", SKY_CUBE_MAP),
    std::make_pair("SAMPLER_SCENE_MODEL", SCENE_TEMPLE),
    std::make_pair("SAMPLER_SCENE_SKY", SCENE_SKY),
    std::make_pair("SSBO_TEMPLE_VERTEX", TEMPLE_VERTEX),
    std::make_pair("SSBO_SKY_VERTEX", SKY_VERTEX),
    std::make_pair("SSBO_LIGHT_DATA", LIGHT_DATA),
    std::make_pair("UBO_MATRIX", MATRIX),
  }};
};
#endif //TEMPLEGL_SRC_RENDERER_H_
