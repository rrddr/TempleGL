#include "camera.h"

const float HALF_PI = 1.57079632679f;

Camera::Camera(glm::vec3 position, float yaw, float pitch, float speed, float max_speed,
               float fov, float aspect_ratio, float near_plane, float far_plane)
    : front_(glm::vec3(0.0f, 0.0f, -1.0f)), world_up_(glm::vec3(0.0f, 1.0f, 0.0f)),
      move_speed_(speed), max_move_speed_(max_speed), position_(position), yaw_(yaw), pitch_(pitch),
      fov_(fov), aspect_ratio_(aspect_ratio), near_plane_(near_plane), far_plane_(far_plane) {
  updateCameraVectors(); // 'up_' and 'right_' fields are initialized here
}

void Camera::processKeyboard(CameraMoveDirection direction, float delta_time) {
  float units_moved = move_speed_ * delta_time;
  switch (direction) {
    case FORWARD:
      position_ += front_ * units_moved;
      break;
    case BACKWARD:
      position_ -= front_ * units_moved;
      break;
    case LEFT:
      position_ -= right_ * units_moved;
      break;
    case RIGHT:
      position_ += right_ * units_moved;
      break;
    case UP:
      position_.y += units_moved;
      break;
    case DOWN:
      position_.y -= units_moved;
      break;
  }
}

void Camera::processMouseMovement(float x_offset, float y_offset) {
  yaw_ += x_offset * 0.005f;
  pitch_ += y_offset * 0.005f;
  pitch_ = glm::clamp(pitch_, -HALF_PI + 0.01f, HALF_PI - 0.01f);
  updateCameraVectors();
}

void Camera::processMouseScroll(float offset, float delta_time) {
  move_speed_ += offset * delta_time * 50;
  move_speed_ = glm::clamp(move_speed_, 0.1f, max_move_speed_);
}

void Camera::updateCameraVectors() {
  glm::vec3 new_front;
  new_front.x = cos(yaw_) * cos(pitch_);
  new_front.y = sin(pitch_);
  new_front.z = sin(yaw_) * cos(pitch_);
  front_ = glm::normalize(new_front);
  right_ = glm::normalize(glm::cross(front_, world_up_));
  up_ = glm::normalize(glm::cross(right_, front_));
}