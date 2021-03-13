#pragma once

#include <functional>
#include "../Engine/Graphics/GlfwManager.hpp"
#include "editor.h"
#include "EditorView.h"

namespace Terrain { namespace Engine { namespace Interop {
    class ViewportContext
    {
        Graphics::GlfwManager *glfw;
        GLFWwindow *window;

        std::function<void()> onRenderCallback;
        EditorView editorView;
        void *viewState;

        uint32 viewportX;
        uint32 viewportY;

    public:
        void *imgBuffer;

        ViewportContext(Graphics::GlfwManager *glfw,
            void *imgBuffer,
            std::function<void()> onRenderCallback);
        ~ViewportContext();

        EditorViewContext getViewContext() const;
        EditorView getEditorView() const;
        bool isDetached() const;

        void render();
        void resize(uint32 x, uint32 y, uint32 width, uint32 height, void *buffer);
        void makePrimary();
        void makeCurrent();
        void setEditorView(EditorView editorView);
        void setViewState(void *viewState);

        void detach();
        void reattach(void *imgBuffer, std::function<void()> onRenderCallback);
    };
}}}