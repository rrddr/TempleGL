#include "camera.h"

Camera::Camera(glm::vec3 position, float yaw, float pitch, float speed, float max_speed,
               float fov, float aspect_ratio, float near_plane, float far_plane)
    : config_ {fov, aspect_ratio, near_plane, far_plane, max_speed},
      state_ {position, {}, {}, {}, yaw, pitch, speed} {
  updateCameraVectors(); // front, right, and up initialized here based on yaw and pitch
}

void Camera::processKeyboard(MoveDirection direction, float delta_time) {
  float units_moved = state_.move_speed * delta_time;
  switch (direction) {
    case FORWARD:
      state_.position += state_.front * units_moved;
      break;
    case BACKWARD:
      state_.position -= state_.front * units_moved;
      break;
    case LEFT:
      state_.position -= state_.right * units_moved;
      break;
    case RIGHT:
      state_.position += state_.right * units_moved;
      break;
    case UP:
      state_.position.y += units_moved;
      break;
    case DOWN:
      state_.position.y -= units_moved;
      break;
  }
}

void Camera::processMouseMovement(float x_offset, float y_offset) {
  state_.yaw += x_offset * 0.005f;
  state_.pitch += y_offset * 0.005f;
  state_.pitch = glm::clamp(state_.pitch, -HALF_PI + 0.01f, HALF_PI - 0.01f);
  updateCameraVectors();
}

void Camera::processMouseScroll(float offset, float delta_time) {
  state_.move_speed += offset * delta_time * 50;
  state_.move_speed = glm::clamp(state_.move_speed, 0.1f, config_.max_move_speed);
}

void Camera::updateCameraVectors() {
  glm::vec3 new_front;
  new_front.x = cos(state_.yaw) * cos(state_.pitch);
  new_front.y = sin(state_.pitch);
  new_front.z = sin(state_.yaw) * cos(state_.pitch);
  state_.front = glm::normalize(new_front);
  state_.right = glm::normalize(glm::cross(state_.front, WORLD_UP));
  state_.up = glm::normalize(glm::cross(state_.right, state_.front));
}