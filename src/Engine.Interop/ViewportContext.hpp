#pragma once

#include "../Engine/Graphics/Window.hpp"
#include "editor.h"
#include "EditorView.h"

namespace Terrain { namespace Engine { namespace Interop {
    class ViewportContext
    {
        Graphics::Window window;
        char *imgBuffer;
        std::function<void()> onRenderCallback;
        EditorView editorView;
        void *viewState;

        uint32 viewportX;
        uint32 viewportY;

    public:
        ViewportContext(Graphics::GlfwManager &glfw,
            char *imgBuffer,
            std::function<void()> onRenderCallback);

        EditorViewContext getViewContext() const;
        EditorView getEditorView() const;
        bool isDetached() const;

        void render();
        void resize(uint32 x, uint32 y, uint32 width, uint32 height, char *buffer);
        void makePrimary();
        void makeCurrent();
        void setEditorView(EditorView editorView);
        void setViewState(void *viewState);

        void detach();
        void reattach(char *imgBuffer, std::function<void()> onRenderCallback);
    };
}}}