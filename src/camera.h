#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 *  Abstract camera object that can be positioned and oriented in world space. Also provides methods for computing
 *  view and projection matrices.
 */
class Camera {
 public:
  explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                  float yaw = 0.0f,
                  float pitch = 0.0f,
                  float speed = 1.0f,
                  float max_speed = 30.0f,
                  float fov = 1.5708f,
                  float aspect_ratio = 1.7778f,
                  float near_plane = 0.1f,
                  float far_plane = 50.0f);

  inline glm::mat4 getViewMatrix() const {
    return glm::lookAt(state_.position,
                       state_.position + state_.front,
                       state_.up);
  }
  inline glm::mat4 getProjectionMatrix() const {
    return glm::perspective(config_.fov,
                            config_.aspect_ratio,
                            config_.near_plane,
                            config_.far_plane);
  }
  inline void updateAspectRatio(float aspect_ratio) { config_.aspect_ratio = aspect_ratio; }

  enum MoveDirection { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

  /**
   * Changes the camera position in world space.
   *
   * @param direction   Desired direction to move in. FORWARD, BACKWARD, LEFT, and RIGHT are interpreted relative to
   *                    camera orientation. UP and DOWN refer to world space y-axis.
   * @param delta_time  Time interval during which movement occurs. In practice: time it took to render this frame.
   */
  void processKeyboard(MoveDirection direction, float delta_time);

  /**
   * Changes the camera orientation based on mouse movement. Pitch is clamped between -PI/2 and PI/2.
   *
   * @param x_offset    Amount of horizontal mouse movement. Affects yaw.
   * @param y_offset    Amount of vertical mouse movement. Affects pitch.
   */
  void processMouseMovement(float x_offset, float y_offset);

  /**
   * Changes the internal move_speed parameter based on scroll wheel input.
   *
   * @param offset  Amount that the wheel was scrolled.
   * @param delta_time  Time interval during which change occurs. In practice: time it took to render this frame.
   */
  void processMouseScroll(float offset, float delta_time);

 private:
  struct Config {
    float fov;
    float aspect_ratio;
    float near_plane;
    float far_plane;
    float max_move_speed;
  };
  struct State {
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    float yaw;
    float pitch;
    float move_speed;
  };
  Config config_ {};
  State state_ {};

  static constexpr glm::vec3 WORLD_UP {0.0f, 1.0f, 0.0f};
  static constexpr float HALF_PI {1.57079632679f};

  void updateCameraVectors();
};

#endif //CAMERA_H_
