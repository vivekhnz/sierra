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

namespace Terrain { namespace Engine {
    class EXPORT Scene
    {
        EngineContext &ctx;
        World &world;

        Graphics::MeshRenderer meshRenderer;
        Terrain terrain;
        Graphics::Camera orbitCamera;
        Graphics::Camera playerCamera;
        IO::InputManager input;

        Graphics::Texture heightmapTexture;

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

        int quadMeshInstanceHandle;
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
        void toggleCameraMode();

        ~Scene();
    };
}}

#endif