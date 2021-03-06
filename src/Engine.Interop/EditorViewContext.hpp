#pragma once

#include "../Engine/terrain_platform.h"

namespace Terrain { namespace Engine { namespace Interop {
    struct EditorViewContext
    {
        uint32 contextId;
        uint32 width;
        uint32 height;
    };
}}}