#ifndef GRAPHICS_SHADERMANAGER_HPP
#define GRAPHICS_SHADERMANAGER_HPP

#include "../Common.hpp"
#include <string>

namespace Terrain { namespace Engine { namespace Graphics {
    class Renderer;

    class EXPORT ShaderManager
    {
        int loadFromFile(unsigned int shaderType, std::string filePath) const;

        Renderer &renderer;

    public:
        ShaderManager(Renderer &renderer);
        ShaderManager(const ShaderManager &that) = delete;
        ShaderManager &operator=(const ShaderManager &that) = delete;
        ShaderManager(ShaderManager &&) = delete;
        ShaderManager &operator=(ShaderManager &&) = delete;

        int loadVertexShaderFromFile(std::string filePath) const;
        int loadTessControlShaderFromFile(std::string filePath) const;
        int loadTessEvalShaderFromFile(std::string filePath) const;
        int loadFragmentShaderFromFile(std::string filePath) const;
        int loadComputeShaderFromFile(std::string filePath) const;

        ~ShaderManager();
    };
}}}

#endif