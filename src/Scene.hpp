#ifndef SCENE_HPP
#define SCENE_HPP

#include "Graphics/Window.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "Graphics/Camera.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/Texture.hpp"
#include "IO/InputManager.hpp"
#include "Terrain.hpp"

class Scene
{
    Window &window;
    Terrain terrain;
    Camera floatingCamera;
    Camera playerCamera;
    InputManager input;

    float orbitAngle;
    float orbitDistance;
    float lightAngle;
    float prevFrameTime;

    glm::vec3 playerLookDir;
    float playerCameraYaw;
    float playerCameraPitch;
    bool isFloatingCameraMode;

    void updateFloatingCamera(float deltaTime);
    void updatePlayerCamera(float deltaTime);
    void onMouseMove(float xOffset, float yOffset);

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