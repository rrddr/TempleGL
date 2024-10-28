#include "camera.h"

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), world_up(glm::vec3(0.0f, 1.0f, 0.0f)),
      movement_speed(SPEED), mouse_sensitivity(SENSITIVITY),
      position(position), yaw(yaw), pitch(pitch) {
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
  yaw += x_offset * mouse_sensitivity;
  pitch += y_offset * mouse_sensitivity;
  pitch = glm::clamp(pitch, -HALF_PI + 0.01f, HALF_PI - 0.01f);
  updateCameraVectors();
}

void Camera::processMouseScroll(float offset, float delta_time) {
  movement_speed += (float) offset * delta_time;
  movement_speed = glm::clamp(movement_speed, 0.2f, 30.0f);
}

void Camera::updateCameraVectors() {
  glm::vec3 new_front;
  new_front.x = (float) (cos(yaw) * cos(pitch));
  new_front.y = (float) sin(pitch);
  new_front.z = (float) (sin(yaw) * cos(pitch));
  front = glm::normalize(new_front);
  right = glm::normalize(glm::cross(front, world_up));
  up = glm::normalize(glm::cross(right, front));
}