#ifndef SCENE_HPP
#define SCENE_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "World.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Mesh.hpp"
#include "IO/InputManager.hpp"
#include "EngineViewContext.hpp"
#include "Terrain.hpp"

namespace Terrain { namespace Engine {
    class EXPORT Scene
    {
        EngineContext &ctx;
        World &world;

        Graphics::Texture heightmapTexture;
        Terrain terrain;

        bool isLightingEnabled;
        bool isTextureEnabled;
        bool isNormalMapEnabled;
        bool isDisplacementMapEnabled;
        bool isAOMapEnabled;
        bool isRoughnessMapEnabled;

        Graphics::Mesh quadMesh;
        Graphics::ShaderProgram quadShaderProgram;

    public:
        Scene(EngineContext &ctx, World &world);
        Scene(const Scene &that) = delete;
        Scene &operator=(const Scene &that) = delete;
        Scene(Scene &&) = delete;
        Scene &operator=(Scene &&) = delete;

        Terrain &getTerrain();

        void draw(EngineViewContext &vctx);
        void toggleLighting();
        void toggleAlbedoMap();
        void toggleNormalMap();
        void toggleDisplacementMap();
        void toggleAmbientOcclusionMap();
        void toggleRoughnessMap();

        ~Scene();
    };
}}

#endif