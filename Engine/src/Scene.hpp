#ifndef SCENE_HPP
#define SCENE_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "World.hpp"

namespace Terrain { namespace Engine {
    class EXPORT Scene
    {
        EngineContext &ctx;
        World &world;

        int terrain_terrainRendererInstanceId;

    public:
        Scene(EngineContext &ctx, World &world);

        void loadTerrainHeightmapFromFile(std::string path);
        void toggleTerrainWireframeMode();
    };
}}

#endif