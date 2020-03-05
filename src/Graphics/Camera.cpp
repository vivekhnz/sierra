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

glm::vec3 Camera::getRotation() const
{
    return rotation;
}
void Camera::setRotation(glm::vec3 newRot)
{
    rotation = newRot;
}

glm::mat4 Camera::getMatrix() const
{
    auto [windowWidth, windowHeight] = window.getSize();
    const float aspectRatio = (float)windowWidth / (float)windowHeight;
    glm::mat4 transform = glm::perspective(fov, aspectRatio, near, far);
    transform = glm::rotate(transform, rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::rotate(transform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::translate(transform, -position);
}

Camera::~Camera()
{
}