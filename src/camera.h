#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float HALF_PI = 1.57079632679f;
namespace { //default values
  const float YAW = -HALF_PI;
  const float PITCH = 0.0f;
  const float SPEED = 15.0f;
  const float SENSITIVITY = 0.01f;
}

enum CameraMoveDirection { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

/**
 *  Abstract camera object that can be positioned and oriented in world space. Provides methods to control these
 *  values based on user input, as well as a method to compute view space transform matrix.
 */
class Camera {
 public:
  /**
   * Simple constructor.
   *
   * @param position    Optional initial position in world space. Default value is origin.
   * @param yaw         Optional initial yaw in radians. Default value is PI/2.
   * @param pitch       Optional initial pitch in radians. Default value is 0.
   */
  explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

  /**
   * Computes transform matrix from world space to view space.
   */
  inline glm::mat4 getViewMatrix() const { return glm::lookAt(position, position + front, up); }

  /**
   * Changes the camera position in world space. Amount of movement is determined by internal move_speed parameter and
   * the delta_time argument.
   *
   * @param direction   Desired direction to move in. FORWARD, BACKWARD, LEFT, and RIGHT are interpreted relative to
   *                    camera orientation. UP and DOWN refer to world space y-axis.
   * @param delta_time  Time interval during which movement occurs. In practice: time it took to render this frame.
   */
  void processKeyboard(CameraMoveDirection direction, float delta_time);

  /**
   * Changes the camera orientation based on mouse movement. Pitch is clamped between -PI/2 and PI/2.
   *
   * @param x_offset    Amount of horizontal mouse movement. Affects yaw.
   * @param y_offset    Amount of vertical mouse movement. Affects pitch.
   */
  void processMouseMovement(float x_offset, float y_offset);

  /**
   * Changes the internal movement_speed parameter based on scroll wheel input.
   *
   * @param offset  Amount that the wheel was scrolled during the time interval indicated by delta_time argument.
   * @param delta_time  Time interval during which change occurs. In practice: time it took to render this frame.
   */
  void processMouseScroll(float offset, float delta_time);

 private:
  glm::vec3 position, front, up, right, world_up;
  float yaw, pitch, movement_speed, mouse_sensitivity;

  void updateCameraVectors();
};

#endif //CAMERA_H_
