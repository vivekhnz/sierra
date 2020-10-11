#include "GraphicsAssetManager.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    GraphicsAssetManager::GraphicsAssetManager(Renderer &renderer) : renderer(renderer)
    {
    }

    int GraphicsAssetManager::createMaterial(int shaderProgramResourceId,
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

        return materials.count++;
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