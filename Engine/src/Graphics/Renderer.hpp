#ifndef GRAPHICS_RENDERER_HPP
#define GRAPHICS_RENDERER_HPP

#include "../Common.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>

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
            std::vector<unsigned int> id;
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

        struct VertexArrays
        {
            int count;
            std::vector<unsigned int> id;

            VertexArrays() : count(0)
            {
            }
        } vertexArrays;

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

            std::map<std::pair<unsigned int, std::string>, unsigned int> uniformNameToLocation;
            std::map<int, int> resourceIdToHandle;

            ShaderPrograms() : count(0)
            {
            }
        } shaderPrograms;

        struct Framebuffers
        {
            int count;
            std::vector<unsigned int> id;

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
            int isRoughnessMapEnabled;
        };

        Renderer();
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
            Resources::TextureResourceData *data);
        void onTextureReloaded(Resources::TextureResourceData &resource);
        void bindTextures(int *textureHandles, int count);
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

        int createVertexArray();
        void bindVertexArray(int handle);

        void onShadersLoaded(const int count, Resources::ShaderResource *resources);

        void onShaderProgramsLoaded(
            const int count, Resources::ShaderProgramResource *resources);
        void useShaderProgram(int handle);
        void setPolygonMode(int polygonMode);
        void setShaderProgramUniformFloat(int handle, std::string uniformName, float value);
        void setShaderProgramUniformInt(int handle, std::string uniformName, int value);
        void setShaderProgramUniforms(int handle,
            int uniformCount,
            int uniformOffset,
            const std::vector<std::string> &uniformNames,
            const std::vector<UniformValue> &uniformValues);
        int lookupShaderProgram(int resourceId)
        {
            return shaderPrograms.resourceIdToHandle[resourceId];
        }

        int createFramebuffer(int textureHandle);

        void clearBackBuffer(
            int width, int height, glm::vec4 clearColor, int framebufferHandle);

        ~Renderer();
    };
}}}

#endif