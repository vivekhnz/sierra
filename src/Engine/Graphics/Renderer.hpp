#ifndef GRAPHICS_RENDERER_HPP
#define GRAPHICS_RENDERER_HPP

#include "../Common.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>

#include "../terrain_foundation.h"
#include "../Resources/TextureResource.hpp"
#include "../Resources/ShaderResource.hpp"
#include "../Resources/ShaderProgramResource.hpp"
#include "UniformValue.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Renderer
    {
    private:
        static const int UNIFORM_BUFFER_COUNT = 2;
        struct UniformBuffers
        {
            unsigned int id[UNIFORM_BUFFER_COUNT];
            unsigned int size[UNIFORM_BUFFER_COUNT];

            UniformBuffers() : id(), size()
            {
            }
        } uniformBuffers;

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

        struct VertexBuffers
        {
            int count;
            std::vector<unsigned int> id;
            std::vector<unsigned int> usage;

            VertexBuffers() : count(0)
            {
            }
        } vertexBuffers;

        struct ElementBuffers
        {
            int count;
            std::vector<unsigned int> id;
            std::vector<unsigned int> usage;

            ElementBuffers() : count(0)
            {
            }
        } elementBuffers;

        struct Shaders
        {
            int count;
            std::vector<unsigned int> id;

            std::map<int, int> resourceIdToHandle;

            Shaders() : count(0)
            {
            }
        } shaders;

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
        enum class UniformBuffer : unsigned int
        {
            Camera = 0,
            Lighting = 1
        };

        struct CameraState
        {
            glm::mat4 transform;
        };
        struct LightingState
        {
            glm::vec4 lightDir;
            int isEnabled;
            int isTextureEnabled;
            int isNormalMapEnabled;
            int isAOMapEnabled;
            int isDisplacementMapEnabled;
        };

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

        EngineMemory *memory;

        Renderer(EngineMemory *memory);
        Renderer(const Renderer &that) = delete;
        Renderer &operator=(const Renderer &that) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        void initialize();
        void updateUniformBuffer(UniformBuffer buffer, void *data);

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

        int createVertexBuffer(unsigned int usage);
        void updateVertexBuffer(int handle, int size, const void *data);
        unsigned int getVertexBufferId(int handle) const;

        int createElementBuffer(unsigned int usage);
        void updateElementBuffer(int handle, int size, const void *data);
        unsigned int getElementBufferId(int handle) const;

        void onShadersLoaded(const int count, Resources::ShaderResource *resources);

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