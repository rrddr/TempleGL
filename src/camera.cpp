#include "camera.h"

const float HALF_PI = 1.57079632679f;

Camera::Camera(glm::vec3 position, float yaw, float pitch, float speed, float max_speed)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), world_up(glm::vec3(0.0f, 1.0f, 0.0f)),
      move_speed(speed), max_move_speed(max_speed), position(position), yaw(yaw), pitch(pitch) {
  updateCameraVectors(); // 'up' and 'right' fields are initialized here
}

void Camera::processKeyboard(CameraMoveDirection direction, float delta_time) {
  float units_moved = move_speed * delta_time;
  switch (direction) {
    case FORWARD:
      position += front * units_moved;
      break;
    case BACKWARD:
      position -= front * units_moved;
      break;
    case LEFT:
      position -= right * units_moved;
      break;
    case RIGHT:
      position += right * units_moved;
      break;
    case UP:
      position.y += units_moved;
      break;
    case DOWN:
      position.y -= units_moved;
      break;
  }
}

void Camera::processMouseMovement(float x_offset, float y_offset) {
  yaw += x_offset * 0.005f;
  pitch += y_offset * 0.005f;
  pitch = glm::clamp(pitch, -HALF_PI + 0.01f, HALF_PI - 0.01f);
  updateCameraVectors();
}

void Camera::processMouseScroll(float offset, float delta_time) {
  move_speed += offset * delta_time * 50;
  move_speed = glm::clamp(move_speed, 0.1f, max_move_speed);
}

void Camera::updateCameraVectors() {
  glm::vec3 new_front;
  new_front.x = cos(yaw) * cos(pitch);
  new_front.y = sin(pitch);
  new_front.z = sin(yaw) * cos(pitch);
  front = glm::normalize(new_front);
  right = glm::normalize(glm::cross(front, world_up));
  up = glm::normalize(glm::cross(right, front));
}