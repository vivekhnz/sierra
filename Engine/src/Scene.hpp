#ifndef SCENE_HPP
#define SCENE_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "World.hpp"
#include "Graphics/Mesh.hpp"
#include "Terrain.hpp"

namespace Terrain { namespace Engine {
    class EXPORT Scene
    {
        Terrain terrain;

        Graphics::Mesh quadMesh;

    public:
        Scene(EngineContext &ctx, World &world);

        Terrain &getTerrain();
    };
}}

#endif