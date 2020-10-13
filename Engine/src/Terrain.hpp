#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "Common.hpp"
#include "World.hpp"

namespace Terrain { namespace Engine {
    class EXPORT Terrain
    {
        int terrainRendererInstanceId;

        EngineContext &ctx;
        World &world;

    public:
        Terrain(EngineContext &ctx, World &world);

        void loadHeightmapFromFile(std::string path);
        void loadHeightmap(Resources::TextureResource &resource);

        void toggleWireframeMode();
    };
}}

#endif