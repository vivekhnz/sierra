#include "HostedEngineViewContext.hpp"

#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    HostedEngineViewContext::HostedEngineViewContext(Graphics::GlfwManager &glfw,
        char *imgBuffer,
        std::function<void()> onRenderCallback,
        int id) :
        window(glfw, 1280, 720, "Terrain", true),
        imgBuffer(imgBuffer), viewportX(0), viewportY(0), onRenderCallback(onRenderCallback),
        hasFocus(false), isHovered(false), id(id)
    {
    }

    int HostedEngineViewContext::getId() const
    {
        return id;
    }

    std::tuple<int, int> HostedEngineViewContext::getViewportSize() const
    {
        return window.getSize();
    }

    std::tuple<int, int> HostedEngineViewContext::getViewportPos() const
    {
        return std::make_tuple(viewportX, viewportY);
    }

    bool HostedEngineViewContext::isDetached() const
    {
        return imgBuffer == nullptr;
    }

    void HostedEngineViewContext::render()
    {
        if (imgBuffer != nullptr)
        {
            window.readPixels(imgBuffer);
        }
        window.refresh();
        if (onRenderCallback != nullptr)
        {
            onRenderCallback();
        }
    }

    void HostedEngineViewContext::resize(int x, int y, int width, int height, char *buffer)
    {
        viewportX = x;
        viewportY = y;
        imgBuffer = buffer;
        window.setSize(width, height);
    }

    void HostedEngineViewContext::makePrimary()
    {
        window.makePrimary();
    }

    void HostedEngineViewContext::makeCurrent()
    {
        window.makeCurrent();
    }

    void HostedEngineViewContext::detach()
    {
        imgBuffer = nullptr;
        onRenderCallback = nullptr;
    }

    void HostedEngineViewContext::reattach(
        char *imgBuffer, std::function<void()> onRenderCallback)
    {
        this->imgBuffer = imgBuffer;
        this->onRenderCallback = onRenderCallback;
    }

    HostedEngineViewContext::~HostedEngineViewContext()
    {
    }
}}}