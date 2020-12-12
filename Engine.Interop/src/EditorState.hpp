#pragma once

#include <glm/glm.hpp>

namespace Terrain { namespace Engine { namespace Interop {
public
    enum class EditStatus : int
    {
        Initializing = 0,
        Idle = 1,
        Editing = 2,
        Committing = 3,
        Discarding = 4
    };

    struct EditorState
    {
        EditStatus editStatus;
        glm::vec2 currentBrushPos;
    };
}}}