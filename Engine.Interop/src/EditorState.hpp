#pragma once

#include <glm/glm.hpp>

namespace Terrain { namespace Engine { namespace Interop {
public
    enum class HeightmapStatus : int
    {
        Initializing = 0,
        Idle = 1,
        Editing = 2,
        Committing = 3,
        Discarding = 4
    };

    enum class InteractionMode : int
    {
        MoveCamera = 0,
        ModifyBrushRadius = 1,
        ModifyBrushFalloff = 2,
        PaintBrushStroke = 3
    };

    struct EditorState
    {
        HeightmapStatus heightmapStatus;
        InteractionMode mode;
        glm::vec2 currentBrushPos;
        float brushRadius;
        float brushFalloff;
    };
}}}