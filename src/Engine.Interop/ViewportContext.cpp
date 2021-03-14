#include "ViewportContext.hpp"

#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    ViewportContext::ViewportContext(HDC deviceContext) :
        deviceContext(deviceContext), viewportX(0), viewportY(0), viewportWidth(0),
        viewportHeight(0), viewState(0), editorView(EditorView::None)
    {
    }

    EditorViewContext ViewportContext::getViewContext() const
    {
        EditorViewContext result = {};
        result.viewState = viewState;
        result.x = viewportX;
        result.y = viewportY;
        result.width = (uint32)viewportWidth;
        result.height = (uint32)viewportHeight;

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
        return deviceContext == nullptr;
    }

    void ViewportContext::render()
    {
        SwapBuffers(deviceContext);
    }

    void ViewportContext::resize(uint32 x, uint32 y, uint32 width, uint32 height)
    {
        viewportX = x;
        viewportY = y;
        viewportWidth = width;
        viewportHeight = height;
    }

    void ViewportContext::detach()
    {
        deviceContext = 0;
    }

    void ViewportContext::reattach(HDC deviceContext)
    {
        this->deviceContext = deviceContext;
    }
}}}