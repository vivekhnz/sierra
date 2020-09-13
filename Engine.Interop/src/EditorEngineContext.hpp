#pragma once

#include "../../Engine/src/EngineContext.hpp"
#include "HostedEngineViewContext.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class EditorEngineContext : public EngineContext
    {
        int prevMousePosX;
        int prevMousePosY;
        double mouseOffsetX;
        double mouseOffsetY;
        double nextMouseScrollOffsetX;
        double nextMouseScrollOffsetY;
        double mouseScrollOffsetX;
        double mouseScrollOffsetY;
        bool isMouseCaptured;
        bool wasMouseCaptured;

    public:
        EditorEngineContext();
        EditorEngineContext(const EditorEngineContext &that) = delete;
        EditorEngineContext &operator=(const EditorEngineContext &that) = delete;
        EditorEngineContext(EditorEngineContext &&) = delete;
        EditorEngineContext &operator=(EditorEngineContext &&) = delete;

        bool isKeyPressed(int key) const;
        bool isMouseButtonPressed(int button) const;
        std::tuple<double, double> getMouseOffset() const;
        std::tuple<double, double> getMouseScrollOffset() const;

        void setMouseCaptureMode(bool shouldCaptureMouse);
        void handleInput();
        void exit();

        void onMouseScroll(double x, double y);
        bool isInMouseCaptureMode() const;

        ~EditorEngineContext();
    };
}}}