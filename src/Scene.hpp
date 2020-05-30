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
    Camera floatingCamera;
    Camera playerCamera;
    Mesh mesh;
    Texture heightmapTexture;
    Texture terrainAlbedoTexture;
    Texture terrainNormalTexture;
    Texture terrainDisplacementTexture;
    Texture terrainAOTexture;
    Texture terrainRoughnessTexture;
    Buffer tessellationLevelBuffer;
    InputManager input;

    float orbitAngle;
    float orbitDistance;
    float lightAngle;
    float prevFrameTime;
    int meshEdgeCount;

    glm::vec3 playerLookDir;
    float playerCameraYaw;
    float playerCameraPitch;

    std::vector<float> terrainHeights;
    int terrainColumns;
    int terrainRows;
    float terrainPatchSize;

    bool isLightingEnabled;
    bool isTextureEnabled;
    bool isNormalMapEnabled;
    bool isDisplacementMapEnabled;
    bool isAOMapEnabled;
    bool isRoughnessMapEnabled;
    bool isWireframeMode;
    bool isFloatingCameraMode;

    void updateFloatingCamera(float deltaTime);
    void updatePlayerCamera(float deltaTime);
    void onMouseMove(float xOffset, float yOffset);
    float getTerrainHeight(float worldX, float worldZ);
    float getTerrainPatchHeight(int x, int z);

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