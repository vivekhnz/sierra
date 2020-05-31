#ifndef SCENE_HPP
#define SCENE_HPP

#include "Graphics/Window.hpp"
#include "Graphics/Camera.hpp"
#include "IO/InputManager.hpp"
#include "Terrain.hpp"

class Scene
{
    Window &window;
    Terrain terrain;
    Camera orbitCamera;
    Camera playerCamera;
    InputManager input;

    float lightAngle;
    float prevFrameTime;
    bool isOrbitCameraMode;

    float orbitYAngle;
    float orbitXAngle;
    float orbitDistance;
    glm::vec3 orbitLookAt;
    bool wasManipulatingCamera;

    glm::vec3 playerLookDir;
    float playerCameraYaw;
    float playerCameraPitch;

    void toggleCameraMode();
    void updateOrbitCamera(float deltaTime);
    void updatePlayerCamera(float deltaTime);
    void onMouseMove(float xOffset, float yOffset);
    void onMouseScroll(float xOffset, float yOffset);

public:
    Scene(Window &window);
    Scene(const Scene &that) = delete;
    Scene &operator=(const Scene &that) = delete;
    Scene(Scene &&) = delete;
    Scene &operator=(Scene &&) = delete;

    void update();
    void draw();

    ~Scene();
};

#endif