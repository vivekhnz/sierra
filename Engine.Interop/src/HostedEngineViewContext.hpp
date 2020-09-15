#pragma once

#include "../../Engine/src/EngineViewContext.hpp"
#include "../../Engine/src/Graphics/Window.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class HostedEngineViewContext : public EngineViewContext
    {
        Graphics::Window window;
        char *imgBuffer;
        std::function<void()> onRenderCallback;
        int cameraEntityId;
        int inputControllerId;

        std::tuple<int, int> location;
        bool hasFocus;
        bool isHovered;

    public:
        HostedEngineViewContext(Graphics::GlfwManager &glfw,
            char *imgBuffer,
            std::function<void()> onRenderCallback);
        HostedEngineViewContext(const HostedEngineViewContext &that) = delete;
        HostedEngineViewContext &operator=(const HostedEngineViewContext &that) = delete;
        HostedEngineViewContext(HostedEngineViewContext &&) = delete;
        HostedEngineViewContext &operator=(HostedEngineViewContext &&) = delete;

        int getCameraEntityId() const;
        int getInputControllerId() const;
        ViewportDimensions getViewportSize() const;
        std::tuple<int, int> getViewportLocation() const;
        bool isDetached() const;

        void render();
        void resize(int x, int y, int width, int height, char *buffer);
        void makePrimary();
        void makeCurrent();
        void setCameraEntityId(int cameraEntityId);
        void setInputControllerId(int cameraEntityId);

        void detach();
        void reattach(char *imgBuffer, std::function<void()> onRenderCallback);

        ~HostedEngineViewContext();
    };
}}}