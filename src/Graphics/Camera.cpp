#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const Window &window)
    : window(window), near(0.1f), far(10000.0f), fov(glm::pi<float>() / 4.0f),
      up(glm::vec3(0.0f, 1.0f, 0.0f))
{
}

glm::vec3 Camera::getPosition() const
{
    return position;
}
void Camera::setPosition(glm::vec3 newPos)
{
    position = newPos;
}

void Camera::lookAt(glm::vec3 lookAtPos)
{
    target = lookAtPos;
}

glm::mat4 Camera::getMatrix() const
{
    auto [windowWidth, windowHeight] = window.getSize();
    const float aspectRatio = (float)windowWidth / (float)windowHeight;
    return glm::perspective(fov, aspectRatio, near, far) * glm::lookAt(position, target, up);
}

Camera::~Camera()
{
}