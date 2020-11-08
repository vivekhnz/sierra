#ifndef GRAPHICS_DEBUGUIRENDERER_HPP
#define GRAPHICS_DEBUGUIRENDERER_HPP

#include "../Common.hpp"

#include "GraphicsAssetManager.hpp"
#include "Renderer.hpp"
#include "../EngineViewContext.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT DebugUIRenderer
    {
    private:
        GraphicsAssetManager &graphicsAssets;
        Renderer &renderer;

        int quadMeshHandle;

        struct Points
        {
            int count;
            int capacity;
            std::vector<glm::vec3> position;

            Points() : count(0), capacity(0)
            {
            }
        } points;

    public:
        DebugUIRenderer(GraphicsAssetManager &graphicsAssets, Renderer &renderer);

        void onMaterialsLoaded(const int count, Resources::MaterialResource *resources);
        void drawPoint(float ndc_x, float ndc_y);
        void render(EngineViewContext &vctx);
    };
}}}

#endif