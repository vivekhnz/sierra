#ifndef GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP
#define GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP

#include "../Common.hpp"
#include "../ResourceManager.hpp"
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

    public:
        MeshRendererComponentManager(ResourceManager &resourceMgr);
        MeshRendererComponentManager(const MeshRendererComponentManager &that) = delete;
        MeshRendererComponentManager &operator=(
            const MeshRendererComponentManager &that) = delete;
        MeshRendererComponentManager(MeshRendererComponentManager &&) = delete;
        MeshRendererComponentManager &operator=(MeshRendererComponentManager &&) = delete;

        int create(int entityId, int meshHandle, int materialHandle);

        int getMeshHandle(int i)
        {
            return data.meshHandle[i];
        }
        int getMaterialHandle(int i)
        {
            return data.materialHandle[i];
        }

        void renderMeshes();

        ~MeshRendererComponentManager();
    };
}}}

#endif