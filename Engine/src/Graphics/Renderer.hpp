#ifndef GRAPHICS_RENDERER_HPP
#define GRAPHICS_RENDERER_HPP

#include "../Common.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>

#include "../Resources/TextureResource.hpp"
#include "../Resources/ShaderResource.hpp"

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

            ShaderPrograms() : count(0)
            {
            }
        } shaderPrograms;

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

        void onTexturesLoaded(const int count, Resources::TextureResource *resources);
        void updateTexture(int handle, int width, int height, const void *pixels);
        void bindTextures(int *textureHandles, int count);
        int lookupTexture(int resourceId)
        {
            return textures.resourceIdToHandle[resourceId];
        }

        int createVertexBuffer(unsigned int usage);
        void updateVertexBuffer(int handle, int size, const void *data);
        unsigned int getVertexBufferId(int handle) const;

        void onShadersLoaded(const int count, Resources::ShaderResource *resources);
        int lookupShader(int resourceId)
        {
            return shaders.resourceIdToHandle[resourceId];
        }

        int createShaderProgram(const std::vector<int> &shaderHandles,
            const std::vector<std::string> &uniformNames);
        unsigned int getShaderProgramId(int handle) const;
        void setShaderProgramUniformMat4(
            int handle, std::string uniformName, bool transpose, glm::mat4 matrix);
        void setShaderProgramUniformFloat(int handle, std::string uniformName, float value);
        void setShaderProgramUniformInt(int handle, std::string uniformName, int value);
        void setShaderProgramUniformVector2(
            int handle, std::string uniformName, glm::vec2 value);
        void setShaderProgramUniformVector3(
            int handle, std::string uniformName, glm::vec3 value);

        ~Renderer();
    };
}}}

#endif