#ifndef GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP
#define GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP

#include "../Common.hpp"
#include "../ResourceManager.hpp"
#include "Renderer.hpp"
#include <vector>

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT MeshRendererComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<int> meshHandle;
            std::vector<int> materialHandle;

            ComponentData() : count(0)
            {
            }
        } data;

        ResourceManager &resourceMgr;
        Graphics::Renderer &renderer;

    public:
        MeshRendererComponentManager(
            ResourceManager &resourceMgr, Graphics::Renderer &renderer);
        MeshRendererComponentManager(const MeshRendererComponentManager &that) = delete;
        MeshRendererComponentManager &operator=(
            const MeshRendererComponentManager &that) = delete;
        MeshRendererComponentManager(MeshRendererComponentManager &&) = delete;
        MeshRendererComponentManager &operator=(MeshRendererComponentManager &&) = delete;

        int create(int entityId, int meshHandle, int materialHandle);

        void renderMeshes();

        void setMaterialHandle(int i, int materialHandle)
        {
            data.materialHandle[i] = materialHandle;
        }

        ~MeshRendererComponentManager();
    };
}}}

#endif