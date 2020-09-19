#include "MeshRendererComponentManager.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRendererComponentManager::MeshRendererComponentManager()
    {
    }

    int MeshRendererComponentManager::create(int entityId, int meshHandle, int materialHandle)
    {
        data.entityId.push_back(entityId);
        data.meshHandle.push_back(meshHandle);
        data.materialHandle.push_back(materialHandle);
        return data.count++;
    }

    MeshRendererComponentManager::~MeshRendererComponentManager()
    {
        data.count = 0;
    }
}}}