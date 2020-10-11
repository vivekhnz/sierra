#include "GraphicsAssetManager.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    GraphicsAssetManager::GraphicsAssetManager(Renderer &renderer) : renderer(renderer)
    {
    }

    void GraphicsAssetManager::onMaterialsLoaded(
        const int count, Resources::MaterialResource *resources)
    {
        for (int i = 0; i < count; i++)
        {
            Resources::MaterialResource &resource = resources[i];

            materials.shaderProgramHandle.push_back(
                renderer.lookupShaderProgram(resource.shaderProgramResourceId));
            materials.polygonMode.push_back(resource.polygonMode);

            materials.firstTextureIndex.push_back(materials.textureHandles.size());
            materials.textureCount.push_back(resource.textureCount);
            for (int t = 0; t < resource.textureCount; t++)
            {
                materials.textureHandles.push_back(
                    renderer.lookupTexture(resource.textureResourceIds[t]));
            }

            int currentUniformCount = materials.uniformNames.size();
            int newUniformCount = currentUniformCount + resource.uniformCount;
            materials.firstUniformIndex.push_back(currentUniformCount);
            materials.uniformCount.push_back(resource.uniformCount);
            materials.uniformNames.resize(newUniformCount);
            materials.uniformValues.resize(newUniformCount);

            int uniformNameStart = 0;
            for (int u = 0; u < resource.uniformCount; u++)
            {
                int idx = currentUniformCount + u;

                int uniformNameLength = resource.uniformNameLengths[u];
                char *uniformName = new char[uniformNameLength + 1];
                memcpy(
                    uniformName, &resource.uniformNames[uniformNameStart], uniformNameLength);
                uniformName[uniformNameLength] = '\0';
                uniformNameStart += uniformNameLength;

                materials.uniformNames[idx] = uniformName;
                materials.uniformValues[idx] = resource.uniformValues[u];
                delete[] uniformName;
            }

            materials.resourceIdToHandle[resource.id] = materials.count++;
        }
    }

    void GraphicsAssetManager::createMaterial(int resourceId,
        int shaderProgramResourceId,
        int polygonMode,
        std::vector<int> textureResourceIds,
        std::vector<std::string> uniformNames,
        std::vector<UniformValue> uniformValues)
    {
        materials.shaderProgramHandle.push_back(
            renderer.lookupShaderProgram(shaderProgramResourceId));
        materials.polygonMode.push_back(polygonMode);

        int textureCount = textureResourceIds.size();
        materials.firstTextureIndex.push_back(materials.textureHandles.size());
        materials.textureCount.push_back(textureCount);
        for (int i = 0; i < textureCount; i++)
        {
            materials.textureHandles.push_back(renderer.lookupTexture(textureResourceIds[i]));
        }

        int uniformCount = uniformNames.size();
        materials.firstUniformIndex.push_back(materials.uniformNames.size());
        materials.uniformCount.push_back(uniformCount);
        materials.uniformNames.insert(
            materials.uniformNames.end(), uniformNames.begin(), uniformNames.end());
        materials.uniformValues.insert(
            materials.uniformValues.end(), uniformValues.begin(), uniformValues.end());

        materials.resourceIdToHandle[resourceId] = materials.count++;
    }

    int &GraphicsAssetManager::getMaterialShaderProgramHandle(int handle)
    {
        return materials.shaderProgramHandle[handle];
    }

    void GraphicsAssetManager::useMaterial(int handle)
    {
        int &shaderProgramHandle = materials.shaderProgramHandle[handle];

        renderer.useShaderProgram(shaderProgramHandle);
        renderer.setPolygonMode(materials.polygonMode[handle]);
        renderer.bindTextures(
            materials.textureHandles.data() + materials.firstTextureIndex[handle],
            materials.textureCount[handle]);

        renderer.setShaderProgramUniforms(shaderProgramHandle, materials.uniformCount[handle],
            materials.firstUniformIndex[handle], materials.uniformNames,
            materials.uniformValues);
    }
}}}