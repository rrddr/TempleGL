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
  void renderSunlightCSM();

  /// Callbacks
  void framebufferSizeCallback(int width, int height) override;
  void cursorPosCallback(float x_pos, float y_pos) override;
  void scrollCallback(float y_offset) override;

  /// Helper methods
  [[nodiscard]] glm::mat4 getSunlightMatrixForCascade(float near_plane, float far_plane) const;
  static std::vector<glm::vec4> getFrustumCorners(const glm::mat4& projection, const glm::mat4& view);
  static void checkFramebufferErrors(const wrap::Framebuffer& framebuffer);

  /// Hardcoded shader parameters
  struct Light {
    glm::vec4 source;
    glm::vec4 color;
    float intensity;
    float padding[3];   // padding to conform with std430 storage layout rules
  };
  static constexpr Light SUNLIGHT {{-0.4f, 0.9f, -1.0f, 0.0f},
                                   {1.0f, 0.7f, 0.4f, 1.0f},
                                   3.0f};
  static constexpr Light DEFAULT_POINT_LIGHT {{0.0f, 0.0f, 0.0f, 1.0f},
                                              {0.6f, 1.0f, 0.9f, 1.0f},
                                              0.0125f};
  static constexpr GLsizei CSM_TEX_SIZE {16192};
  static constexpr size_t CSM_NUM_CASCADES {3};

  static constexpr size_t SCENE_FBO_NUM_COLOR_ATTACHMENTS {2};
  static constexpr size_t SCENE_FBO_COLOR_INDEX_TEMPLE {0};
  static constexpr size_t SCENE_FBO_COLOR_INDEX_SKY {1};

  static constexpr GLuint TEX_BINDING_TEMPLE_ARRAY {0};
  static constexpr GLuint TEX_BINDING_SKY_CUBE_MAP {1};
  static constexpr GLuint TEX_BINDING_SCENE_TEMPLE {2};
  static constexpr GLuint TEX_BINDING_SCENE_SKY {3};
  static constexpr GLuint TEX_BINDING_CSM_ARRAY {4};

  static constexpr GLuint SSBO_BINDING_TEMPLE_VERTICES {0};
  static constexpr GLuint SSBO_BINDING_SKY_VERTICES {1};
  static constexpr GLuint SSBO_BINDING_LIGHT_DATA {2};

  static constexpr GLuint UBO_BINDING_MATRIX {0};
};
#endif //TEMPLEGL_SRC_RENDERER_H_
