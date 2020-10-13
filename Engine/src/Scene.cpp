#include "Scene.hpp"

#include "TerrainResources.hpp"
#include "Graphics/Window.hpp"
#include "IO/Path.hpp"

namespace Terrain { namespace Engine {
    Scene::Scene(EngineContext &ctx, World &world) : terrain(ctx, world)
    {
        terrain.initialize();

        // configure input
        ctx.input.mapCommand(GLFW_KEY_Z, std::bind(&Terrain::toggleWireframeMode, &terrain));
        ctx.input.mapCommand(GLFW_KEY_H,
            std::bind(&Terrain::loadHeightmapFromFile, &terrain,
                IO::Path::getAbsolutePath("data/heightmap2.tga")));

        // setup heightmap quad
        std::vector<float> quadVertices(20);

        quadVertices[0] = 0.0f;
        quadVertices[1] = 0.0f;
        quadVertices[2] = 0.0f;
        quadVertices[3] = 0.0f;
        quadVertices[4] = 0.0f;

        quadVertices[5] = 1.0f;
        quadVertices[6] = 0.0f;
        quadVertices[7] = 0.0f;
        quadVertices[8] = 1.0f;
        quadVertices[9] = 0.0f;

        quadVertices[10] = 1.0f;
        quadVertices[11] = 1.0f;
        quadVertices[12] = 0.0f;
        quadVertices[13] = 1.0f;
        quadVertices[14] = 1.0f;

        quadVertices[15] = 0.0f;
        quadVertices[16] = 1.0f;
        quadVertices[17] = 0.0f;
        quadVertices[18] = 0.0f;
        quadVertices[19] = 1.0f;

        std::vector<unsigned int> quadIndices(6);
        quadIndices[0] = 0;
        quadIndices[1] = 2;
        quadIndices[2] = 1;
        quadIndices[3] = 0;
        quadIndices[4] = 3;
        quadIndices[5] = 2;

        int quadMesh_meshHandle =
            ctx.assets.graphics.createMesh(GL_TRIANGLES, quadVertices, quadIndices);

        int quadMesh_entityId = ctx.entities.create();
        world.componentManagers.meshRenderer.create(quadMesh_entityId, quadMesh_meshHandle,
            TerrainResources::RESOURCE_ID_MATERIAL_QUAD, std::vector<std::string>(),
            std::vector<Graphics::UniformValue>());
    }

    Terrain &Scene::getTerrain()
    {
        return terrain;
    }
}}