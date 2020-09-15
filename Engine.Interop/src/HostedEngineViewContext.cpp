#include "HostedEngineViewContext.hpp"

#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    HostedEngineViewContext::HostedEngineViewContext(
        Graphics::GlfwManager &glfw, char *imgBuffer, std::function<void()> onRenderCallback) :
        window(glfw, 1280, 720, "Terrain", true),
        imgBuffer(imgBuffer), onRenderCallback(onRenderCallback), location(0, 0),
        hasFocus(false), isHovered(false), cameraEntityId(-1), inputControllerId(-1)
    {
    }

    int HostedEngineViewContext::getCameraEntityId() const
    {
        return cameraEntityId;
    }
    void HostedEngineViewContext::setCameraEntityId(int cameraEntityId)
    {
        this->cameraEntityId = cameraEntityId;
    }

    int HostedEngineViewContext::getInputControllerId() const
    {
        return inputControllerId;
    }
    void HostedEngineViewContext::setInputControllerId(int inputControllerId)
    {
        this->inputControllerId = inputControllerId;
    }

    ViewportDimensions HostedEngineViewContext::getViewportSize() const
    {
        auto [w, h] = window.getSize();
        return {w, h};
    }

    std::tuple<int, int> HostedEngineViewContext::getViewportLocation() const
    {
        return location;
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
        location = std::make_tuple(x, y);
        window.setSize(width, height);
        imgBuffer = buffer;
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