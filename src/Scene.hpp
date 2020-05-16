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
    ShaderProgram terrainShaderProgram;
    ShaderProgram wireframeShaderProgram;
    ShaderProgram calcTessLevelsShaderProgram;
    Camera camera;
    Mesh mesh;
    Texture heightmapTexture;
    Texture terrainAlbedoTexture;
    Texture terrainNormalTexture;
    Texture terrainDisplacementTexture;
    Buffer tessellationLevelBuffer;
    InputManager input;

    float orbitAngle;
    float orbitDistance;
    float prevFrameTime;
    int meshEdgeCount;

    bool isLightingEnabled;
    bool isTextureEnabled;
    bool isNormalMapEnabled;
    bool isDisplacementMapEnabled;
    bool isWireframeMode;

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