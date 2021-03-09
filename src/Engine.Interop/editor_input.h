#ifndef EDITOR_INPUT_H
#define EDITOR_INPUT_H

#include <glm/gtc/type_ptr.hpp>
#include "../Engine/terrain_platform.h"

#define PLATFORM_CAPTURE_MOUSE(name) void name(bool retainCursorPos)
typedef PLATFORM_CAPTURE_MOUSE(PlatformCaptureMouse);

struct EditorInput
{
    void *activeViewState;

    float scrollOffset;
    glm::vec2 normalizedCursorPos;
    glm::vec2 cursorOffset;
    uint8 pressedMouseButtons;
    uint8 prevPressedMouseButtons;

    uint64 pressedKeys;
    uint64 prevPressedKeys;

    PlatformCaptureMouse *platformCaptureMouse;
};

#endif