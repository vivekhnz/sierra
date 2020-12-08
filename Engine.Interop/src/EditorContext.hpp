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
            std::vector<IO::KeyboardInputState> keyboard;

            InputState() : count(0)
            {
            }
        } inputState;

    public:
        EditorContext();

        // input
        void updateInputState();
        IO::MouseInputState getMouseState(int inputControllerId) const;
        IO::KeyboardInputState getKeyboardState(int inputControllerId) const;
        void setMouseCaptureMode(bool shouldCaptureMouse);

        int addInputController();
        void onMouseScroll(double x, double y);
        bool isInMouseCaptureMode() const;
    };
}}}