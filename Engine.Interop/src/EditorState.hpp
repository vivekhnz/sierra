#pragma once

namespace Terrain { namespace Engine { namespace Interop {
public
    enum class EditStatus : int
    {
        Initializing = 0,
        Idle = 1,
        Editing = 2,
        Committing = 3
    };

    struct EditorState
    {
        EditStatus editStatus;
        float brushQuadX;
        float brushQuadY;
    };
}}}