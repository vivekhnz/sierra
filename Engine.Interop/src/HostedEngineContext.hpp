#pragma once

#include "../../Engine/src/EngineContext.hpp"
#include "../../Engine/src/Graphics/Window.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class HostedEngineContext : public EngineContext
    {
        Graphics::GlfwManager glfw;
        Graphics::Window window;

        System::DateTime startTime;

        char *imgBuffer;

    public:
        HostedEngineContext(char *imgBuffer);
        HostedEngineContext(const HostedEngineContext &that) = delete;
        HostedEngineContext &operator=(const HostedEngineContext &that) = delete;
        HostedEngineContext(HostedEngineContext &&) = delete;
        HostedEngineContext &operator=(HostedEngineContext &&) = delete;

        float getCurrentTime() const;
        std::tuple<int, int> getViewportSize() const;
        bool isKeyPressed(int key) const;
        bool isMouseButtonPressed(int button) const;

        void addMouseMoveHandler(std::function<void(double, double)> handler);
        void addMouseScrollHandler(std::function<void(double, double)> handler);
        void setMouseCaptureMode(bool shouldCaptureMouse);
        void render();
        void exit();

        void setViewportSize(int width, int height);
        void setBuffer(char *buffer);

        ~HostedEngineContext();
    };
}}}