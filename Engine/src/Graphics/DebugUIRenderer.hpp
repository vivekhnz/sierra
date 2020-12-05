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
            static const int MAX_POINTS = 1024;
            static const int STRIDE = 2 * sizeof(float);
            static const int SIZE = MAX_POINTS * STRIDE;

            int instanceBufferHandle;
            float instanceBufferData[MAX_POINTS * 2];

            int count;

            Points() : count(0)
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