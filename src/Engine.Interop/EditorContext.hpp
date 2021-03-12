#pragma once

#include "ViewportContext.hpp"
#include "editor.h"

namespace Terrain { namespace Engine { namespace Interop {
    class EditorContext
    {
        glm::vec2 prevMousePos;
        glm::vec2 capturedMousePos;
        float nextMouseScrollOffsetY;
        bool shouldCaptureMouse;
        bool wasMouseCaptured;

    public:
        EditorContext();

        void getInputState(EditorInput *input);
        void setMouseCaptureMode(bool shouldCaptureMouse);

        void onMouseScroll(double x, double y);
    };
}}}