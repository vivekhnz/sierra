#ifndef GRAPHICS_UNIFORMVALUE_HPP
#define GRAPHICS_UNIFORMVALUE_HPP

#include "../Common.hpp"

#include <glm/glm.hpp>

namespace Terrain { namespace Engine { namespace Graphics {
    enum class UniformType : unsigned int
    {
        Float = 0,
        Integer = 1,
        Vector2 = 2,
        Vector3 = 3,
        Matrix4x4 = 4
    };

    struct UniformValue
    {
        UniformType type;
        union
        {
            float f;
            int i;
            glm::vec2 vec2;
            glm::vec3 vec3;
            glm::mat4 mat4;
        };

        static UniformValue forFloat(float value)
        {
            Graphics::UniformValue u;
            u.type = UniformType::Float;
            u.f = value;
            return u;
        }

        static UniformValue forInteger(int value)
        {
            Graphics::UniformValue u;
            u.type = UniformType::Integer;
            u.i = value;
            return u;
        }

        static UniformValue forVector2(glm::vec2 value)
        {
            Graphics::UniformValue u;
            u.type = UniformType::Vector2;
            u.vec2 = value;
            return u;
        }

        static UniformValue forVector3(glm::vec3 value)
        {
            Graphics::UniformValue u;
            u.type = UniformType::Vector3;
            u.vec3 = value;
            return u;
        }

        static UniformValue forMatrix4x4(glm::mat4 value)
        {
            Graphics::UniformValue u;
            u.type = UniformType::Matrix4x4;
            u.mat4 = value;
            return u;
        }
    };
}}}

#endif