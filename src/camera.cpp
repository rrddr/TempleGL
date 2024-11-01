#include "camera.h"

const float HALF_PI = 1.57079632679f;

Camera::Camera(glm::vec3 position, float yaw, float pitch, float speed)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), world_up(glm::vec3(0.0f, 1.0f, 0.0f)),
      movement_speed(speed), position(position), yaw(yaw), pitch(pitch) {
  updateCameraVectors(); // 'up' and 'right' fields are initialized here
}

void Camera::processKeyboard(CameraMoveDirection direction, float delta_time) {
  float velocity = movement_speed * delta_time;
  switch (direction) {
    case FORWARD:
      position += front * velocity;
      break;
    case BACKWARD:
      position -= front * velocity;
      break;
    case LEFT:
      position -= right * velocity;
      break;
    case RIGHT:
      position += right * velocity;
      break;
    case UP:
      position.y += velocity;
      break;
    case DOWN:
      position.y -= velocity;
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
  movement_speed += offset * delta_time * 50;
  movement_speed = glm::clamp(movement_speed, 0.1f, 30.0f);
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