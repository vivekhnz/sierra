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

        std::tuple<int, int> location;
        bool hasFocus;
        bool isHovered;

    public:
        ViewportContext(Graphics::GlfwManager &glfw,
            char *imgBuffer,
            std::function<void()> onRenderCallback);

        EditorViewContext getViewContext() const;
        EditorView getEditorView() const;
        std::tuple<int, int> getViewportLocation() const;
        bool isDetached() const;

        void render();
        void resize(int x, int y, int width, int height, char *buffer);
        void makePrimary();
        void makeCurrent();
        void setEditorView(EditorView editorView);
        void setViewState(void *viewState);

        void detach();
        void reattach(char *imgBuffer, std::function<void()> onRenderCallback);
    };
}}}