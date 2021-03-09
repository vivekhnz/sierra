#include "ViewportContext.hpp"

#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    ViewportContext::ViewportContext(
        Graphics::GlfwManager &glfw, char *imgBuffer, std::function<void()> onRenderCallback) :
        window(glfw, 1280, 720, "Terrain", true),
        imgBuffer(imgBuffer), onRenderCallback(onRenderCallback), location(0, 0),
        hasFocus(false), isHovered(false), viewState(0), world(Worlds::ViewportWorld::None)
    {
    }

    EditorViewContext ViewportContext::getViewContext() const
    {
        auto [w, h] = window.getSize();

        EditorViewContext result = {};
        result.width = w;
        result.height = h;
        result.viewState = viewState;
        return result;
    }

    Worlds::ViewportWorld ViewportContext::getWorld() const
    {
        return world;
    }
    void ViewportContext::setWorld(Worlds::ViewportWorld world)
    {
        this->world = world;
    }

    void ViewportContext::setViewState(void *viewState)
    {
        this->viewState = viewState;
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