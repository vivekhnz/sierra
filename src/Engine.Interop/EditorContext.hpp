#pragma once

#include "../Engine/IO/MouseCaptureMode.hpp"
#include "../Engine/IO/MouseInputState.hpp"
#include "ViewportContext.hpp"
#include "editor_input.h"

namespace Terrain { namespace Engine { namespace Interop {
    class EditorContext
    {
        int prevMousePosX;
        int prevMousePosY;
        double nextMouseScrollOffsetX;
        double nextMouseScrollOffsetY;
        IO::MouseCaptureMode currentMouseCaptureMode;
        IO::MouseCaptureMode prevMouseCaptureMode;
        int capturedMousePosX;
        int capturedMousePosY;

        IO::MouseInputState mouseState;
        uint8 pressedMouseButtons;
        uint64 pressedKeys;

    public:
        void* activeViewState;

        EditorContext();

        void updateInputState();
        void getInputState(EditorInput *input);
        void setMouseCaptureMode(IO::MouseCaptureMode mode);

        void onMouseScroll(double x, double y);
    };
}}}