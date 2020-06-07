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
        std::function<void(double, double)> onMouseMoveHandler;
        std::function<void(double, double)> onMouseScrollHandler;

        int viewportWidth;
        int viewportHeight;

        int prevMousePosX;
        int prevMousePosY;
        bool isMouseCaptured;
        bool isFirstCapturedMouseInput;
        bool isMouseInBounds;

        void handleInput(System::Windows::Point viewportPos, System::Windows::Point mousePos);

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
        void render(System::Windows::Point viewportPos, System::Windows::Point mousePos);
        void exit();

        void setViewportSize(int width, int height);
        void setBuffer(char *buffer);
        void setIsMouseInBounds(bool value);
        void onMouseMove(double x, double y);
        void onMouseScroll(double x, double y);

        ~HostedEngineContext();
    };
}}}