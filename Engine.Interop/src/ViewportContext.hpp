#pragma once

#include "../../Engine/src/EngineViewContext.hpp"
#include "../../Engine/src/Graphics/Window.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class ViewportContext
    {
        Graphics::Window window;
        char *imgBuffer;
        std::function<void()> onRenderCallback;
        int worldId;
        int cameraEntityId;
        int inputControllerId;

        std::tuple<int, int> location;
        bool hasFocus;
        bool isHovered;

    public:
        ViewportContext(Graphics::GlfwManager &glfw,
            char *imgBuffer,
            std::function<void()> onRenderCallback);

        EngineViewContext getViewContext() const;
        int getInputControllerId() const;
        int getWorldId() const;
        std::tuple<int, int> getViewportLocation() const;
        bool isDetached() const;

        void render();
        void resize(int x, int y, int width, int height, char *buffer);
        void makePrimary();
        void makeCurrent();
        void setWorldId(int worldId);
        void setCameraEntityId(int cameraEntityId);
        void setInputControllerId(int cameraEntityId);

        void detach();
        void reattach(char *imgBuffer, std::function<void()> onRenderCallback);
    };
}}}