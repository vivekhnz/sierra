#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const Window &window)
    : window(window), near(0.1f), far(100.0f), fov(glm::pi<float>() / 4.0f)
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

glm::mat4 Camera::getMatrix() const
{
    auto [windowWidth, windowHeight] = window.getSize();
    const float aspectRatio = (float)windowWidth / (float)windowHeight;
    glm::mat4 transform = glm::perspective(fov, aspectRatio, near, far);
    return glm::translate(transform, position);
}

Camera::~Camera()
{
}