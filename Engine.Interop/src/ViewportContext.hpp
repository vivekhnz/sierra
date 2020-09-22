#pragma once

#include "../../Engine/src/EngineViewContext.hpp"
#include "../../Engine/src/Graphics/Window.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class ViewportContext
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
        ViewportContext(Graphics::GlfwManager &glfw,
            char *imgBuffer,
            std::function<void()> onRenderCallback);
        ViewportContext(const ViewportContext &that) = delete;
        ViewportContext &operator=(const ViewportContext &that) = delete;
        ViewportContext(ViewportContext &&) = delete;
        ViewportContext &operator=(ViewportContext &&) = delete;

        EngineViewContext getViewContext() const;
        int getInputControllerId() const;
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

        ~ViewportContext();
    };
}}}