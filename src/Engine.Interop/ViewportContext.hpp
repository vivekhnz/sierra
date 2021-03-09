#pragma once

#include "../Engine/Graphics/Window.hpp"
#include "editor.h"
#include "Worlds/ViewportWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class ViewportContext
    {
        Graphics::Window window;
        char *imgBuffer;
        std::function<void()> onRenderCallback;
        Worlds::ViewportWorld world;
        void *viewState;

        std::tuple<int, int> location;
        bool hasFocus;
        bool isHovered;

    public:
        ViewportContext(Graphics::GlfwManager &glfw,
            char *imgBuffer,
            std::function<void()> onRenderCallback);

        EditorViewContext getViewContext() const;
        Worlds::ViewportWorld getWorld() const;
        std::tuple<int, int> getViewportLocation() const;
        bool isDetached() const;

        void render();
        void resize(int x, int y, int width, int height, char *buffer);
        void makePrimary();
        void makeCurrent();
        void setWorld(Worlds::ViewportWorld world);
        void setViewState(void *viewState);

        void detach();
        void reattach(char *imgBuffer, std::function<void()> onRenderCallback);
    };
}}}