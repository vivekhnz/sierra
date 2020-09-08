#ifndef GRAPHICS_CAMERA_HPP
#define GRAPHICS_CAMERA_HPP

#include "../Common.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Terrain { namespace Engine { namespace Graphics { namespace Camera {

    struct EXPORT ViewportDimensions
    {
        float width;
        float height;
    };

    static void calculateMatrices(ViewportDimensions &viewport,
        glm::vec3 *in_cameraPositions,
        glm::vec3 *in_cameraTargets,
        glm::mat4 *out_cameraMatrices,
        int count)
    {
        constexpr float fov = glm::pi<float>() / 4.0f;
        const float nearPlane = 0.1f;
        const float farPlane = 10000.0f;
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        const float aspectRatio = viewport.width / viewport.height;
        glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
        for (int i = 0; i < count; i++)
        {
            out_cameraMatrices[i] =
                projection * glm::lookAt(in_cameraPositions[i], in_cameraTargets[i], up);
        }
    }
}}}}

#endif