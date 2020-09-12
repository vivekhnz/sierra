#ifndef ORBITCAMERACOMPONENTMANAGER_HPP
#define ORBITCAMERACOMPONENTMANAGER_HPP

#include "Common.hpp"
#include <glm/glm.hpp>

namespace Terrain { namespace Engine {
    class EXPORT OrbitCameraComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            int *cameraIndex;
            glm::vec3 *lookAt;
            float *yaw;
            float *pitch;
            float *distance;
        };

        ComponentData data;

    public:
        OrbitCameraComponentManager();
        OrbitCameraComponentManager(const OrbitCameraComponentManager &that) = delete;
        OrbitCameraComponentManager &operator=(
            const OrbitCameraComponentManager &that) = delete;
        OrbitCameraComponentManager(OrbitCameraComponentManager &&) = delete;
        OrbitCameraComponentManager &operator=(OrbitCameraComponentManager &&) = delete;

        void calculateLookAt(float mouseOffsetX,
            float mouseOffsetY,
            float deltaTime,
            glm::vec3 *in_cameraPositions);
        void calculateYawAndPitch(float mouseOffsetX, float mouseOffsetY, float deltaTime);
        void calculateDistance(float scrollY);
        void calculateCameraStates(
            glm::vec3 *out_cameraPositions, glm::vec3 *out_cameraTargets);

        ~OrbitCameraComponentManager();
    };
}}

#endif