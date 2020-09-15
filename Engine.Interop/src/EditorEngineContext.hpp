#pragma once

#include "../../Engine/src/EngineContext.hpp"
#include "HostedEngineViewContext.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class EditorEngineContext : public EngineContext
    {
        int prevMousePosX;
        int prevMousePosY;
        double nextMouseScrollOffsetX;
        double nextMouseScrollOffsetY;
        bool isMouseCaptured;
        bool wasMouseCaptured;

        IO::MouseInputState mouseState;

    public:
        EditorEngineContext();
        EditorEngineContext(const EditorEngineContext &that) = delete;
        EditorEngineContext &operator=(const EditorEngineContext &that) = delete;
        EditorEngineContext(EditorEngineContext &&) = delete;
        EditorEngineContext &operator=(EditorEngineContext &&) = delete;

        bool isKeyPressed(int key) const;

        IO::MouseInputState getMouseState() const
        {
            return mouseState;
        }

        void setMouseCaptureMode(bool shouldCaptureMouse);
        void handleInput();
        void exit();

        void onMouseScroll(double x, double y);
        bool isInMouseCaptureMode() const;

        ~EditorEngineContext();
    };
}}}