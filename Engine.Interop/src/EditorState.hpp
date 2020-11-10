#pragma once

namespace Terrain { namespace Engine { namespace Interop {
    struct EditorState
    {
        float brushQuadX;
        float brushQuadY;
        bool doesHeightmapRequireRedraw;
        bool wasHeightmapUpdated;
        bool shouldDiscardHeightmap;
    };
}}}