#ifndef ORBITCAMERA_HPP
#define ORBITCAMERA_HPP

#include "Common.hpp"

namespace Terrain { namespace Engine { namespace OrbitCamera {

    struct EXPORT OrbitCameraState
    {
        int cameraIndex;
        float xAngle;
        float yAngle;
        glm::vec3 lookAt;
        float distance;
    };

    static void calculateCameraStates(OrbitCameraState *in_orbitCameraStates,
        Graphics::Camera::CameraState *out_cameraStates,
        int count)
    {
        for (int i = 0; i < count; i++)
        {
            OrbitCameraState &orbitCamera = in_orbitCameraStates[i];
            Graphics::Camera::CameraState &camera = out_cameraStates[orbitCamera.cameraIndex];

            float yaw = glm::radians(orbitCamera.yAngle);
            float pitch = glm::radians(orbitCamera.xAngle);
            glm::vec3 lookDir =
                glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
            camera.position = orbitCamera.lookAt + (lookDir * orbitCamera.distance);
            camera.target = orbitCamera.lookAt;
        }
    }
}}}

#endif