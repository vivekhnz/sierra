#pragma once

#include "../Engine/IO/MouseInputState.hpp"
#include "ViewportContext.hpp"
#include "editor.h"

namespace Terrain { namespace Engine { namespace Interop {
    class EditorContext
    {
        int prevMousePosX;
        int prevMousePosY;
        double nextMouseScrollOffsetX;
        double nextMouseScrollOffsetY;
        bool shouldCaptureMouse;
        bool wasMouseCaptured;
        int capturedMousePosX;
        int capturedMousePosY;

        IO::MouseInputState mouseState;
        uint64 pressedButtons;

    public:
        void *activeViewState;

        EditorContext();

        void updateInputState();
        void getInputState(EditorInput *input);
        void setMouseCaptureMode(bool shouldCaptureMouse);

        void onMouseScroll(double x, double y);
    };
}}}