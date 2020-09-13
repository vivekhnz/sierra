#ifndef SCENE_HPP
#define SCENE_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "World.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/MeshRenderer.hpp"
#include "IO/InputManager.hpp"
#include "EngineViewContext.hpp"
#include "Terrain.hpp"

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
        bool isOrbitCameraMode;

        bool wasManipulatingCamera;

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

        void updateOrbitCamera(
            float deltaTime, float mouseOffsetX, float mouseOffsetY, float scrollY);
        void updatePlayerCamera(float deltaTime, float mouseOffsetX, float mouseOffsetY);

    public:
        Scene(EngineContext &ctx, World &world);
        Scene(const Scene &that) = delete;
        Scene &operator=(const Scene &that) = delete;
        Scene(Scene &&) = delete;
        Scene &operator=(Scene &&) = delete;

        Terrain &getTerrain();

        void update(float deltaTime);
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