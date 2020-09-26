#ifndef GRAPHICS_SHADERPROGRAM_HPP
#define GRAPHICS_SHADERPROGRAM_HPP

#include "../Common.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Renderer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT ShaderProgram
    {
        int handle;
        Renderer &renderer;

    public:
        ShaderProgram(Renderer &renderer);

        int getId() const;
        void link(const std::vector<int> &shaderHandles);
        void setMat4(std::string uniformName, bool transpose, glm::mat4 matrix);
        void setFloat(std::string uniformName, float value);
        void setInt(std::string uniformName, int value);
        void setBool(std::string uniformName, bool value);
        void setVector2(std::string uniformName, glm::vec2 value);
        void setVector3(std::string uniformName, glm::vec3 value);
    };
}}}

#endif