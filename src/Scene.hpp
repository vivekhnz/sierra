#ifndef SCENE_HPP
#define SCENE_HPP

#include "Graphics/Window.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "Graphics/Camera.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/Texture.hpp"
#include "IO/InputManager.hpp"

class Scene
{
    Window &window;
    ShaderProgram shaderProgram;
    Camera camera;
    Mesh mesh;
    Texture heightmapTexture;
    Texture terrainTexture;
    InputManager input;

    float orbitAngle;
    float orbitDistance;
    float prevFrameTime;

    bool isLightingEnabled;
    bool isTextureEnabled;
    bool isNormalDisplayEnabled;

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