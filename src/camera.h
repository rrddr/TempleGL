#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


enum CameraMoveDirection { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

/**
 *  Abstract camera object that can be positioned and oriented in world space. Provides methods to control these
 *  values based on user input, as well as a method to compute view space transform matrix.
 */
class Camera {
 public:
  float aspect_ratio_; // needs to be updated on window resize

  explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                  float yaw = 0.0f,
                  float pitch = 0.0f,
                  float speed = 1.0f,
                  float max_speed = 30.0f,
                  float fov = 1.5708f,
                  float aspect_ratio = 1.7778f,
                  float near_plane = 0.1f,
                  float far_plane = 50.0f);

  /**
   * Computes transform matrix from world space to view space.
   */
  inline glm::mat4 getViewMatrix() const { return glm::lookAt(position_, position_ + front_, up_); }

  /**
   * Computes transform matrix from view space to clip space.
   */
  inline glm::mat4 getProjectionMatrix() const { return glm::perspective(fov_, aspect_ratio_, near_plane_, far_plane_); }

  /**
   * Changes the camera position_ in world space.
   *
   * @param direction   Desired direction to move in. FORWARD, BACKWARD, LEFT, and RIGHT are interpreted relative to
   *                    camera orientation. UP and DOWN refer to world space y-axis.
   * @param delta_time  Time interval during which movement occurs. In practice: time it took to render this frame.
   */
  void processKeyboard(CameraMoveDirection direction, float delta_time);

  /**
   * Changes the camera orientation based on mouse movement. Pitch is clamped between -PI/2 and PI/2.
   *
   * @param x_offset    Amount of horizontal mouse movement. Affects yaw_.
   * @param y_offset    Amount of vertical mouse movement. Affects pitch_.
   */
  void processMouseMovement(float x_offset, float y_offset);

  /**
   * Changes the internal move_speed_ parameter based on scroll wheel input.
   *
   * @param offset  Amount that the wheel was scrolled.
   * @param delta_time  Time interval during which change occurs. In practice: time it took to render this frame.
   */
  void processMouseScroll(float offset, float delta_time);

 private:
  // Camera position_ and orientation
  float yaw_, pitch_;
  glm::vec3 position_, front_, up_, right_, world_up_;
  // Camera movement
  float move_speed_, max_move_speed_;
  // View frustum
  float fov_, near_plane_, far_plane_;

  void updateCameraVectors();
};

#endif //CAMERA_H_
