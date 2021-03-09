#pragma once

#include "../Engine/terrain_platform.h"

namespace Terrain { namespace Engine { namespace Interop {
    struct EditorViewContext
    {
        void *viewState;
        uint32 width;
        uint32 height;
    };
}}}