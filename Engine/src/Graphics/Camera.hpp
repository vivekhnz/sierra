#ifndef GRAPHICS_CAMERA_HPP
#define GRAPHICS_CAMERA_HPP

#include "..\Common.hpp"
#include <glm/glm.hpp>
#include "Window.hpp"

class EXPORT Camera
{
    const Window &window;
    float near;
    float far;
    float fov;
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

public:
    Camera(const Window &window);
    Camera(const Camera &that) = delete;
    Camera &operator=(const Camera &that) = delete;
    Camera(Camera &&) = delete;
    Camera &operator=(Camera &&) = delete;

    glm::vec3 getPosition() const;
    void setPosition(glm::vec3 newPos);
    void lookAt(glm::vec3 lookAtPos);
    glm::mat4 getMatrix() const;

    ~Camera();
};

#endif