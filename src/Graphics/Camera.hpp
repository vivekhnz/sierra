#ifndef GRAPHICS_CAMERA_HPP
#define GRAPHICS_CAMERA_HPP

#include <glm/glm.hpp>
#include "Window.hpp"

class Camera
{
    const Window &window;
    float near;
    float far;
    float fov;
    glm::vec3 position;
    glm::vec3 rotation;

public:
    Camera(const Window &window);
    Camera(const Camera &that) = delete;
    Camera &operator=(const Camera &that) = delete;
    Camera(Camera &&) = delete;
    Camera &operator=(Camera &&) = delete;

    glm::vec3 getPosition() const;
    void setPosition(glm::vec3 newPos);
    glm::vec3 getRotation() const;
    void setRotation(glm::vec3 newRot);
    glm::mat4 getMatrix() const;

    ~Camera();
};

#endif