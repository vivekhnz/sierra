#ifndef SCENE_HPP
#define SCENE_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "World.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Camera.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/MeshRenderer.hpp"
#include "IO/InputManager.hpp"
#include "EngineViewContext.hpp"
#include "Terrain.hpp"
#include "OrbitCamera.hpp"

namespace Terrain { namespace Engine {
    class EXPORT Scene
    {
        EngineContext &ctx;
        World &world;

        Graphics::Texture heightmapTexture;

        Graphics::MeshRenderer meshRenderer;
        Terrain terrain;
        IO::InputManager input;

        float lightAngle;
        float prevFrameTime;
        bool isOrbitCameraMode;

        OrbitCamera::OrbitCameraState *orbitCameraStates;
        bool wasManipulatingCamera;

        Graphics::Camera::CameraState *cameraStates;
        glm::mat4 *cameraMatrices;

        glm::vec3 playerLookDir;
        float playerCameraYaw;
        float playerCameraPitch;

        unsigned int cameraUniformBufferId;
        unsigned int lightingUniformBufferId;

        bool isLightingEnabled;
        bool isTextureEnabled;
        bool isNormalMapEnabled;
        bool isDisplacementMapEnabled;
        bool isAOMapEnabled;
        bool isRoughnessMapEnabled;

        Graphics::Mesh quadMesh;
        Graphics::ShaderProgram quadShaderProgram;

        void updateOrbitCamera(float deltaTime);
        void updatePlayerCamera(float deltaTime);
        void onMouseMove(float xOffset, float yOffset);
        void onMouseScroll(float xOffset, float yOffset);

    public:
        Scene(EngineContext &ctx, World &world);
        Scene(const Scene &that) = delete;
        Scene &operator=(const Scene &that) = delete;
        Scene(Scene &&) = delete;
        Scene &operator=(Scene &&) = delete;

        Terrain &getTerrain();

        void update();
        void draw(EngineViewContext &vctx);
        void toggleLighting();
        void toggleAlbedoMap();
        void toggleNormalMap();
        void toggleDisplacementMap();
        void toggleAmbientOcclusionMap();
        void toggleRoughnessMap();
        void toggleCameraMode();

        ~Scene();
    };

    struct LightingState
    {
        glm::vec4 lightDir;
        int isEnabled;
        int isTextureEnabled;
        int isNormalMapEnabled;
        int isAOMapEnabled;
        int isDisplacementMapEnabled;
        int isRoughnessMapEnabled;
    };
}}

#endif