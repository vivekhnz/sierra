#ifndef GRAPHICS_RENDERER_HPP
#define GRAPHICS_RENDERER_HPP

#include "../Common.hpp"
#include "ShaderManager.hpp"
#include "../Resources/TextureResource.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <map>

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

        ShaderManager shaders;

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

        ~Renderer();
    };
}}}

#endif