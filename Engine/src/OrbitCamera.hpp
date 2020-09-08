#ifndef ORBITCAMERA_HPP
#define ORBITCAMERA_HPP

#include "Common.hpp"
#include <algorithm>

namespace Terrain { namespace Engine { namespace OrbitCamera {
    struct EXPORT OrbitCameraState
    {
        int cameraIndex;
        glm::vec3 lookAt;
    };

    struct EXPORT OrbitCameraYawPitch
    {
        float yaw;
        float pitch;
    };

    static void calculateYawAndPitch(float mouseOffsetX,
        float mouseOffsetY,
        float deltaTime,
        float *in_orbitDistance,
        OrbitCameraYawPitch *inout_yawPitch,
        int count)
    {
        for (int i = 0; i < count; i++)
        {
            OrbitCameraYawPitch &yawPitch = inout_yawPitch[i];
            float orbitDistance = in_orbitDistance[i];
            float sensitivity =
                std::max(std::min(orbitDistance * 0.05f, 3.5f), 0.7f) * deltaTime;
            yawPitch.yaw += glm::radians(mouseOffsetX * sensitivity);
            yawPitch.pitch += glm::radians(mouseOffsetY * sensitivity);
        }
    }

    static void calculateCameraStates(OrbitCameraState *in_orbitCameraStates,
        OrbitCameraYawPitch *in_orbitYawPitch,
        float *in_orbitDistance,
        glm::vec3 *out_cameraPositions,
        glm::vec3 *out_cameraTargets,
        int count)
    {
        for (int i = 0; i < count; i++)
        {
            OrbitCameraState &orbitCamera = in_orbitCameraStates[i];
            OrbitCameraYawPitch &yawPitch = in_orbitYawPitch[i];
            float &distance = in_orbitDistance[i];

            glm::vec3 lookDir = glm::vec3(cos(yawPitch.yaw) * cos(yawPitch.pitch),
                sin(yawPitch.pitch), sin(yawPitch.yaw) * cos(yawPitch.pitch));
            out_cameraPositions[orbitCamera.cameraIndex] =
                orbitCamera.lookAt + (lookDir * distance);
            out_cameraTargets[orbitCamera.cameraIndex] = orbitCamera.lookAt;
        }
    }
}}}

#endif