#ifndef GRAPHICS_RENDERER_HPP
#define GRAPHICS_RENDERER_HPP

#include "../Common.hpp"
#include <glm/glm.hpp>
#include "ShaderManager.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Renderer
    {
    private:
        static const int UNIFORM_BUFFER_COUNT = 2;
        unsigned int uniformBufferIds[UNIFORM_BUFFER_COUNT];
        unsigned int uniformBufferSizes[UNIFORM_BUFFER_COUNT];

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

        unsigned int createTexture(int wrapMode, int filterMode);
        void updateTexture(unsigned int id,
            int internalFormat,
            int width,
            int height,
            int format,
            int type,
            const void *pixels);
        void destroyTexture(unsigned int id);

        ~Renderer();
    };
}}}

#endif