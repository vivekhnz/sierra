#pragma once

#include "../../Engine/src/AppContext.hpp"
#include "ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class EditorContext : public AppContext
    {
        int prevMousePosX;
        int prevMousePosY;
        double nextMouseScrollOffsetX;
        double nextMouseScrollOffsetY;
        bool isMouseCaptured;
        bool wasMouseCaptured;

        struct InputState
        {
            int count;
            std::vector<IO::MouseInputState> mouse;

            InputState() : count(0)
            {
            }
        } inputState;

    public:
        EditorContext();

        // input
        void updateInputState();
        bool isKeyPressed(int key) const;
        IO::MouseInputState getMouseState(int inputControllerId) const;
        void setMouseCaptureMode(bool shouldCaptureMouse);

        int addInputController();
        void onMouseScroll(double x, double y);
        bool isInMouseCaptureMode() const;
    };
}}}