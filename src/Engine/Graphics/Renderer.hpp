#ifndef GRAPHICS_RENDERER_HPP
#define GRAPHICS_RENDERER_HPP

#include "../Common.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>

#include "../terrain_foundation.h"
#include "../Resources/TextureResource.hpp"
#include "../Resources/ShaderProgramResource.hpp"
#include "UniformValue.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Renderer
    {
    private:
        struct Textures
        {
            int count;
            std::vector<int> resourceId;
            std::vector<int> internalFormat;
            std::vector<int> format;
            std::vector<int> type;

            std::map<int, int> resourceIdToHandle;

            Textures() : count(0)
            {
            }
        } textures;

        struct ShaderPrograms
        {
            int count;
            std::vector<unsigned int> id;

            std::map<int, int> resourceIdToHandle;

            ShaderPrograms() : count(0)
            {
            }
        } shaderPrograms;

        struct Framebuffers
        {
            int count;
            std::vector<unsigned int> id;
            std::vector<int> textureHandle;

            Framebuffers() : count(0)
            {
            }
        } framebuffers;

    public:
        struct ShaderProgramState
        {
            struct
            {
                int count;
                const char **names;
                UniformValue *values;
            } uniforms;

            struct
            {
                int count;
                int *handles;
            } textures;
        };

        MemoryBlock *memory;

        Renderer(MemoryBlock *memory);
        Renderer(const Renderer &that) = delete;
        Renderer &operator=(const Renderer &that) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        void initialize();

        int createTexture(int width,
            int height,
            int internalFormat,
            int format,
            int type,
            int wrapMode,
            int filterMode);
        void onTexturesLoaded(const int count,
            Resources::TextureResourceDescription *descriptions,
            Resources::TextureResourceUsage *usages,
            Resources::TextureResourceData *data);
        void onTextureReloaded(Resources::TextureResourceData &resource);
        void getTexturePixels(int handle, void *out_data);
        int lookupTexture(int resourceId)
        {
            return textures.resourceIdToHandle[resourceId];
        }

        void onShaderProgramsLoaded(
            const int count, Resources::ShaderProgramResource *resources);
        void useShaderProgram(int handle);
        void setShaderProgramState(int handle, ShaderProgramState &state);
        int lookupShaderProgram(int resourceId)
        {
            return shaderPrograms.resourceIdToHandle[resourceId];
        }

        int createFramebuffer(int textureHandle);

        void useFramebuffer(int handle);
        void finalizeFramebuffer(int handle);

        ~Renderer();
    };
}}}

#endif