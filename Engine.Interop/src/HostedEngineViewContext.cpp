#include "HostedEngineViewContext.hpp"

#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    HostedEngineViewContext::HostedEngineViewContext(Graphics::GlfwManager &glfw,
        char *imgBuffer,
        std::function<void()> onRenderCallback,
        int id) :
        window(glfw, 1280, 720, "Terrain", true),
        imgBuffer(imgBuffer), onRenderCallback(onRenderCallback), hasFocus(false),
        isHovered(false), id(id)
    {
    }

    int HostedEngineViewContext::getId() const
    {
        return id;
    }

    ViewportDimensions HostedEngineViewContext::getViewportSize() const
    {
        auto [w, h] = window.getSize();
        return {w, h};
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

    void HostedEngineViewContext::resize(int width, int height, char *buffer)
    {
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