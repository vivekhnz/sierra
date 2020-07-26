#pragma once

#include "../../Engine/src/EngineViewContext.hpp"
#include "../../Engine/src/Graphics/Window.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class HostedEngineViewContext : public EngineViewContext
    {
        Graphics::Window window;
        char *imgBuffer;
        int viewportX;
        int viewportY;
        std::function<void()> onRenderCallback;
        int id;

        bool hasFocus;
        bool isHovered;

    public:
        HostedEngineViewContext(Graphics::GlfwManager &glfw,
            char *imgBuffer,
            std::function<void()> onRenderCallback,
            int id);
        HostedEngineViewContext(const HostedEngineViewContext &that) = delete;
        HostedEngineViewContext &operator=(const HostedEngineViewContext &that) = delete;
        HostedEngineViewContext(HostedEngineViewContext &&) = delete;
        HostedEngineViewContext &operator=(HostedEngineViewContext &&) = delete;

        int getId() const;
        std::tuple<int, int> getViewportSize() const;
        std::tuple<int, int> getViewportPos() const;
        bool isDetached() const;

        void render();
        void resize(int x, int y, int width, int height, char *buffer);
        void makePrimary();
        void makeCurrent();

        void detach();
        void reattach(char *imgBuffer, std::function<void()> onRenderCallback);

        ~HostedEngineViewContext();
    };
}}}