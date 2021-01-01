#pragma once

#include "../Engine/AppContext.hpp"
#include "ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class EditorContext : public AppContext
    {
        int prevMousePosX;
        int prevMousePosY;
        double nextMouseScrollOffsetX;
        double nextMouseScrollOffsetY;
        IO::MouseCaptureMode currentMouseCaptureMode;
        IO::MouseCaptureMode prevMouseCaptureMode;
        int capturedMousePosX;
        int capturedMousePosY;

        struct InputState
        {
            int count;
            std::vector<IO::MouseInputState> mouse;
            std::vector<unsigned short> pressedMouseButtons;
            std::vector<unsigned long long> pressedKeys;

            InputState() : count(0)
            {
            }
        } inputState;

    public:
        EditorContext();

        // input
        void updateInputState();
        const IO::MouseInputState &getMouseState(int inputControllerId) const;
        const unsigned short &getPressedMouseButtons(int inputControllerId) const;
        const unsigned long long &getPressedKeys(int inputControllerId) const;
        void setMouseCaptureMode(IO::MouseCaptureMode mode);

        int addInputController();
        void onMouseScroll(double x, double y);
        bool isInMouseCaptureMode() const;
    };
}}}