#pragma once

#include "../Engine/EngineViewContext.hpp"
#include "../Engine/Graphics/Window.hpp"
#include "Worlds/ViewportWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class ViewportContext
    {
        Graphics::Window window;
        char *imgBuffer;
        std::function<void()> onRenderCallback;
        Worlds::ViewportWorld world;
        uint32 contextId;
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
        Worlds::ViewportWorld getWorld() const;
        std::tuple<int, int> getViewportLocation() const;
        bool isDetached() const;

        void render();
        void resize(int x, int y, int width, int height, char *buffer);
        void makePrimary();
        void makeCurrent();
        void setWorld(Worlds::ViewportWorld world);
        void setContextId(uint32 contextId);
        void setInputControllerId(int inputControllerId);

        void detach();
        void reattach(char *imgBuffer, std::function<void()> onRenderCallback);
    };
}}}