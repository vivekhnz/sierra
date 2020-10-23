#include "ViewportContext.hpp"

#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    ViewportContext::ViewportContext(
        Graphics::GlfwManager &glfw, char *imgBuffer, std::function<void()> onRenderCallback) :
        window(glfw, 1280, 720, "Terrain", true),
        imgBuffer(imgBuffer), onRenderCallback(onRenderCallback), location(0, 0),
        hasFocus(false), isHovered(false), cameraEntityId(-1), inputControllerId(-1),
        world(Worlds::EditorWorld::None)
    {
    }

    EngineViewContext ViewportContext::getViewContext() const
    {
        auto [w, h] = window.getSize();
        return {w, h, cameraEntityId};
    }

    Worlds::EditorWorld ViewportContext::getWorld() const
    {
        return world;
    }
    void ViewportContext::setWorld(Worlds::EditorWorld world)
    {
        this->world = world;
    }

    void ViewportContext::setCameraEntityId(int cameraEntityId)
    {
        this->cameraEntityId = cameraEntityId;
    }

    int ViewportContext::getInputControllerId() const
    {
        return inputControllerId;
    }
    void ViewportContext::setInputControllerId(int inputControllerId)
    {
        this->inputControllerId = inputControllerId;
    }

    std::tuple<int, int> ViewportContext::getViewportLocation() const
    {
        return location;
    }

    bool ViewportContext::isDetached() const
    {
        return imgBuffer == nullptr;
    }

    void ViewportContext::render()
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

    void ViewportContext::resize(int x, int y, int width, int height, char *buffer)
    {
        location = std::make_tuple(x, y);
        window.setSize(width, height);
        imgBuffer = buffer;
    }

    void ViewportContext::makePrimary()
    {
        window.makePrimary();
    }

    void ViewportContext::makeCurrent()
    {
        window.makeCurrent();
    }

    void ViewportContext::detach()
    {
        imgBuffer = nullptr;
        onRenderCallback = nullptr;
    }

    void ViewportContext::reattach(char *imgBuffer, std::function<void()> onRenderCallback)
    {
        this->imgBuffer = imgBuffer;
        this->onRenderCallback = onRenderCallback;
    }
}}}