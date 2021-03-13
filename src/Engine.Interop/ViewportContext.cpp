#include "ViewportContext.hpp"

#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    ViewportContext::ViewportContext(
        Graphics::GlfwManager *glfw, void *imgBuffer, std::function<void()> onRenderCallback) :
        glfw(glfw),
        imgBuffer(imgBuffer), onRenderCallback(onRenderCallback), viewportX(0), viewportY(0),
        viewState(0), editorView(EditorView::None)
    {
        glfwWindowHint(GLFW_VISIBLE, false);
        window = glfwCreateWindow(1280, 720, "Terrain", NULL, NULL);
        if (!window)
        {
            // todo: log here
        }
    }

    ViewportContext::~ViewportContext()
    {
        glfwDestroyWindow(window);
    }

    EditorViewContext ViewportContext::getViewContext() const
    {
        int32 width;
        int32 height;
        glfwGetWindowSize(window, &width, &height);

        EditorViewContext result = {};
        result.viewState = viewState;
        result.x = viewportX;
        result.y = viewportY;
        result.width = (uint32)width;
        result.height = (uint32)height;

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
        glfwSwapBuffers(window);
        if (onRenderCallback != nullptr)
        {
            onRenderCallback();
        }
    }

    void ViewportContext::resize(uint32 x, uint32 y, uint32 width, uint32 height, void *buffer)
    {
        viewportX = x;
        viewportY = y;
        glfwSetWindowSize(window, width, height);

        imgBuffer = buffer;
    }

    void ViewportContext::makePrimary()
    {
        glfwMakeContextCurrent(window);
        glfw->setPrimaryWindow(window);
    }

    void ViewportContext::makeCurrent()
    {
        glfw->setCurrentWindow(window);
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