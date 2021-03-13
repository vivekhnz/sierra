#include "ViewportContext.hpp"

#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    ViewportContext::ViewportContext(
        Graphics::GlfwManager &glfw, void *imgBuffer, std::function<void()> onRenderCallback) :
        window(glfw, 1280, 720, "Terrain", true),
        imgBuffer(imgBuffer), onRenderCallback(onRenderCallback), viewportX(0), viewportY(0),
        viewState(0), editorView(EditorView::None)
    {
    }

    EditorViewContext ViewportContext::getViewContext() const
    {
        auto [w, h] = window.getSize();

        EditorViewContext result = {};
        result.viewState = viewState;
        result.x = viewportX;
        result.y = viewportY;
        result.width = w;
        result.height = h;

        return result;
    }

    EditorView ViewportContext::getEditorView() const
    {
        return editorView;
    }
    void ViewportContext::setEditorView(EditorView editorView)
    {
        this->editorView = editorView;
    }

    void ViewportContext::setViewState(void *viewState)
    {
        this->viewState = viewState;
    }

    bool ViewportContext::isDetached() const
    {
        return imgBuffer == nullptr;
    }

    void ViewportContext::render()
    {
        window.refresh();
        if (onRenderCallback != nullptr)
        {
            onRenderCallback();
        }
    }

    void ViewportContext::resize(uint32 x, uint32 y, uint32 width, uint32 height, void *buffer)
    {
        viewportX = x;
        viewportY = y;
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

    void ViewportContext::reattach(void *imgBuffer, std::function<void()> onRenderCallback)
    {
        this->imgBuffer = imgBuffer;
        this->onRenderCallback = onRenderCallback;
    }
}}}